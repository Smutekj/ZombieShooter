#include "LuaWrapper.h"
#include "GameWorld.h"



extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"

}

//////////////////////////////////////
// Code for making the singleton work
//////////////////////////////////////

LuaWrapper *LuaWrapper::_instance = 0;

LuaWrapper* 
LuaWrapper::getSingleton()
{
    if (!_instance)
    {
        _instance = new LuaWrapper();
    }
    return _instance;
}


//////////////////////////////////////////////
/// C/C++ functions we are exposing to lua
///////////////////////////////////////////////
int createObject(lua_State *L)
{
    int numArgs = lua_gettop(L);

    if (numArgs != 1)
    {
        return 0;
    }
    const char* object_type = lua_tostring(L,1);
    auto new_obj = GameWorld::getWorld().addObject(object_type);
    new_obj->setPosition({300, 200});
    new_obj->setSize(10);
    return 0;
}


int setPosition(lua_State *L)
{
    int numArgs = lua_gettop(L);

    if (numArgs < 4)
    {
        return 0;
    }
    const char* nodeName = lua_tostring(L,1);
    float x = (float) lua_tonumber(L,2);
    float y = (float) lua_tonumber(L,3);
    // GameWorld::getWorld().setPosition(nodeName, );
    return 0;
}


////////////////////////////////////////////////
///  Registering functions with lua
//////////////////////////////////////////////////

void LuaWrapper::initializeLuaFunctions()
{
    lua_register(mLuaState, "createObject", createObject);
    lua_register(mLuaState, "setPosition", setPosition);

}




void  LuaWrapper::doString(const char *s)
{ 
    luaL_dostring(mLuaState, s); 
}


LuaWrapper::LuaWrapper()
{
    mLuaState = luaL_newstate();
    luaL_openlibs(mLuaState);
    initializeLuaFunctions();
}

LuaWrapper::~LuaWrapper()
{
    lua_close(mLuaState);
}

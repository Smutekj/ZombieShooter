#include "LuaWrapper.h"
#include "GameWorld.h"

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/basic_file_sink.h>

//////////////////////////////////////
//! Code for making the singleton work
//////////////////////////////////////
LuaWrapper *LuaWrapper::s_instance = nullptr;

LuaWrapper *LuaWrapper::getSingleton()
{
    if (!s_instance)
    {
        s_instance = new LuaWrapper();
    }
    return s_instance;
}

LuaWrapper::LuaWrapper()
{
    try
    {
        m_logger = spdlog::basic_logger_mt("lua_logger", "logs/lua-scripts.txt");
        m_logger->flush_on(spdlog::level::err);
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }
    m_lua_state = luaL_newstate();
    luaL_openlibs(m_lua_state);
    initializeLuaFunctions();
}

LuaWrapper::~LuaWrapper()
{
    lua_close(m_lua_state);
}

//////////////////////////////////////////////
/// C/C++ functions we are exposing to lua
///////////////////////////////////////////////
int createObject(lua_State *state)
{
    int num_args = lua_gettop(state);

    if (num_args != 1)
    {
        std::string msg = "OBJECT NOT CREATED! EXPECTED 1 ARGUMENT BUT GOT " + num_args;
        spdlog::get("lua_logger")->error(msg);
        return 0;
    }
    const char *object_type = lua_tostring(state, 1);
    auto new_obj = GameWorld::getWorld().addObject(object_type);
    new_obj->setPosition({300, 200});
    new_obj->setSize(10);
    return 0;
}

int setPosition(lua_State *state)
{
    int num_args = lua_gettop(state);

    if (num_args != 3)
    {
        std::string msg =  "POSITION NOT SET! EXPECTED 2 ARGUMENTs BUT GOT " + num_args;
        spdlog::get("lua_logger")->info(msg);
        return 0;
    }
    const char *nodeName = lua_tostring(state, 1);
    float x = (float)lua_tonumber(state, 2);
    float y = (float)lua_tonumber(state, 3);
    // GameWorld::getWorld().setPosition(nodeName, );
    return 0;
}

//! \brief Registers functions with lua
void LuaWrapper::initializeLuaFunctions()
{
    lua_register(m_lua_state, "createObject", createObject);
    lua_register(m_lua_state, "setPosition" , setPosition);
}

//! \brief runs the command if it is registered
void LuaWrapper::doString(const std::string &command)
{
    luaL_dostring(m_lua_state, command.c_str());
}

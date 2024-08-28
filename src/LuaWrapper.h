//! BOTH OF THESE FILES WERE STOLEN AND ADAPTED FROM HERE

extern "C"
{
#include "lua.h"
}

class GameWorld;


class LuaWrapper
{
public:
    static LuaWrapper *getSingleton();
    void doString(const char *s); 
    ~LuaWrapper();
private:

    static LuaWrapper *_instance;
    LuaWrapper();
    void initializeLuaFunctions();
    lua_State* mLuaState;
};
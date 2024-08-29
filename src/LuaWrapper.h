#pragma once
//! BOTH OF THESE FILES WERE STOLEN AND ADAPTED FROM HERE:


#include <memory>
#include <string>

#ifdef __cplusplus
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#endif

class GameWorld;
namespace spdlog
{
    class logger;
}

class LuaWrapper
{
public:
    static LuaWrapper *getSingleton();
    void doString(const std::string &command);
    ~LuaWrapper();

private:
    static LuaWrapper *s_instance;
    LuaWrapper();
    void initializeLuaFunctions();

private:
    lua_State* m_lua_state;
    std::shared_ptr<spdlog::logger> m_logger;
};
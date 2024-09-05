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

#include <spdlog/sinks/ringbuffer_sink.h>


class GameWorld;
namespace spdlog
{
    class logger;
    // namespace sinks
    // {
    //     class ringbuffer_sink_mt;
    // }
}

class LuaWrapper
{
public:
    static LuaWrapper *getSingleton();
    bool doString(const std::string &command);
    std::string getLastError();
    ~LuaWrapper();

private:
    static LuaWrapper *s_instance;
    LuaWrapper();
    void initializeLuaFunctions();

private:
    lua_State *m_lua_state;
    std::shared_ptr<spdlog::logger> m_logger;
    std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> m_ringbuffer_sink = nullptr;
};
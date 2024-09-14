#pragma once
//! BOTH OF THESE FILES WERE STOLEN AND ADAPTED FROM HERE:

#include <memory>
#include <string>

#ifdef __cplusplus
extern "C"
{
#include "../external/lua/src/lua.h"
#include "../external/lua/src/lualib.h"
#include "../external/lua/src/lauxlib.h"
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
    bool doFile(const std::string &filename);
    // bool runScript(const std::string& script_name, const std::string& function_name);
    std::string getLastError();
    ~LuaWrapper();

    lua_State *m_lua_state;
private:
    static LuaWrapper *s_instance;
    LuaWrapper();
    void initializeLuaFunctions();

private:
    std::shared_ptr<spdlog::logger> m_logger;
    std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> m_ringbuffer_sink = nullptr;
};


// //! \brief runs the \p function in the \p script_filename
// template <class ObjType>
// bool LuaWrapper::runScript(const std::string &script_filename, const std::string &function, ObjType* p_obj)
// {
//     auto file_status = luaL_loadfile(m_lua_state, ("../scripts/" + script_filename).c_str());
//     if (file_status == LUA_ERRFILE)
//     {
//         spdlog::get("lua_logger")->error(" could not load file: " + script_filename + " !");
//         return false;
//     }
//     int scriptLoadStatus = luaL_dofile(m_lua_state, ("../scripts/" + script_filename).c_str());
//     luabridge::LuaRef processFunc = luabridge::getGlobal(m_lua_state, "UpdateAI");
//     if (processFunc.isFunction())
//     {
//         try
//         {
//             processFunc(p_obj);
//         }
//         catch (std::exception e)
//         {
//             std::cout << e.what() << " !\n";
//         }
//     }
//     auto call_status = lua_pcall(m_lua_state, 0, LUA_MULTRET, 0);
//     assert(lua_isinteger(m_lua_state, -1));
//     auto error_status = lua_tointeger(m_lua_state, -1);
//     if (error_status != LUA_OK)
//     {
//         return false;
//     }
//     return true;
// }

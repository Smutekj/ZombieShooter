#include "LuaWrapper.h"
#include "GameWorld.h"
#include "AILuaComponent.h"
#include "Entities.h"

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <LuaBridge/LuaBridge.h>

//! Code for making the singleton work (Likely stolen)
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
        m_ringbuffer_sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(1);

        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/lua_scripts.txt"));
        sinks.push_back(m_ringbuffer_sink);

        m_logger = std::make_shared<spdlog::logger>("lua_logger", sinks.begin(), sinks.end());
        m_logger->flush_on(spdlog::level::err);
        spdlog::register_logger(m_logger);
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
static int createObject(lua_State *state)
{
    int num_args = lua_gettop(state);

    if (num_args != 2)
    {
        std::string msg = " OBJECT NOT CREATED! EXPECTED 2 ARGUMENTS BUT GOT " + std::to_string(num_args);
        spdlog::get("lua_logger")->error(msg);
        lua_pushinteger(state, LUA_ERRRUN);
        lua_error(state);
        return 1;
    }
    const char *object_type = lua_tostring(state, 1);
    const char *object_name = lua_tostring(state, 2);

    auto new_obj = GameWorld::getWorld().addObject(object_type, object_name);
    if (!new_obj)
    {
        lua_pushinteger(state, LUA_ERRRUN);
        lua_error(state);
        return 1;
    }
    new_obj->setPosition({300, 200});
    new_obj->setSize(10);
    lua_pushinteger(state, LUA_OK);
    lua_error(state);
    return 1;
}

static int addEffect(lua_State *state)
{
    int num_args = lua_gettop(state);

    if (num_args != 2)
    {
        std::string msg = " OBJECT NOT CREATED! EXPECTED 2 ARGUMENTS BUT GOT " + std::to_string(num_args);
        spdlog::get("lua_logger")->error(msg);
        lua_pushinteger(state, LUA_ERRRUN);
        lua_error(state);
        return 1;
    }
    const char *object_type = lua_tostring(state, 1);
    const char *object_name = lua_tostring(state, 2);

    auto new_obj = GameWorld::getWorld().addEffect(object_type, object_name);
    if (!new_obj)
    {
        lua_pushinteger(state, LUA_ERRRUN);
        lua_error(state);
        return 1;
    }
    new_obj->setPosition({300, 200});
    new_obj->setSize(10);
    lua_pushinteger(state, LUA_OK);
    lua_error(state);
    return 1;
}

int createObjectAsChild(lua_State *state)
{
    auto logger = spdlog::get("lua_logger");
    int num_args = lua_gettop(state);

    if (num_args != 3)
    {
        std::string msg = " OBJECT NOT CREATED! EXPECTED 3 ARGUMENTS BUT GOT " + std::to_string(num_args);
        logger->error(msg);
        lua_pushinteger(state, LUA_ERRRUN);
        lua_error(state);
        return 1;
    }
    const char *object_type = lua_tostring(state, 1);
    const char *object_name = lua_tostring(state, 2);
    const char *parent_name = lua_tostring(state, 3);

    auto &world = GameWorld::getWorld();
    auto parent_id = world.getIdOf(parent_name);
    auto new_obj = world.addObject(object_type, object_name, parent_id);
    auto parent_obj = world.get(parent_name);
    if (!parent_obj)
    {
        lua_pushinteger(state, LUA_ERRRUN);
        lua_error(state);
        return 1;
    }
    if (!new_obj)
    {
        lua_pushinteger(state, LUA_ERRRUN);
        lua_error(state);
        return 1;
    }
    new_obj->setPosition(parent_obj->getPosition());
    new_obj->setSize(10);
    lua_pushinteger(state, LUA_OK);
    lua_error(state);
    return 1;
}
int changeParentOf(lua_State *state)
{
    int num_args = lua_gettop(state);
    if (num_args != 2)
    {
        std::string msg = " OBJECT NOT CREATED! EXPECTED 2 ARGUMENT BUT GOT " + std::to_string(num_args);
        spdlog::get("lua_logger")->error(msg);
        return 0;
    }
    const char *object_name = lua_tostring(state, 1);
    const char *parent_name = lua_tostring(state, 2);
    auto &world = GameWorld::getWorld();
    auto parent_id = world.getIdOf(parent_name);
    auto my_id = world.getIdOf(object_name);
    if (my_id == -1 || parent_id == -1)
    {
        world.m_scene.changeParentOf(my_id, parent_id);
    }
    return 0;
}

int setPosition(lua_State *state)
{
    int num_args = lua_gettop(state);
    if (num_args != 3)
    {
        std::string msg = "POSITION NOT SET! EXPECTED 3 ARGUMENTS BUT GOT " + std::to_string(num_args);
        spdlog::get("lua_logger")->error(msg);
        lua_pushinteger(state, LUA_ERRRUN);
        lua_error(state);
        return 1;
    }
    const char *entity_name = lua_tostring(state, 1);
    float x = (float)lua_tonumber(state, 2);
    float y = (float)lua_tonumber(state, 3);
    auto &world = GameWorld::getWorld();

    auto entity_id = world.getIdOf(entity_name);
    if (entity_id != -1)
    {
        auto p_entity = world.get(entity_id);
        assert(p_entity);
        p_entity->setPosition({x, y});
        lua_pushinteger(state, LUA_OK);
        lua_error(state);
        return 1;
    }
    lua_pushinteger(state, LUA_ERRRUN);
    lua_error(state);
    return 1;
}
static int setTarget(lua_State *state)
{
    int num_args = lua_gettop(state);
    if (num_args != 3 && num_args != 2)
    {
        std::string msg = "POSITION NOT SET! EXPECTED 3 ARGUMENTS BUT GOT " + std::to_string(num_args);
        spdlog::get("lua_logger")->error(msg);
        lua_pushinteger(state, LUA_ERRRUN);
        lua_error(state);
        return 1;
    }
    auto &world = GameWorld::getWorld();

    const char *entity_name = lua_tostring(state, 1);
    auto p_entity = world.get(entity_name);
    if (num_args == 2)
    {
        const char *target_name = lua_tostring(state, 2);
        if (p_entity)
        {
            p_entity->m_target = world.get(target_name).get();
        }
    }
    else if (num_args == 3)
    {
        float x = (float)lua_tonumber(state, 2);
        float y = (float)lua_tonumber(state, 3);
        if (p_entity)
        {
            p_entity->m_target_pos = {x, y};
        }
    }

    lua_pushinteger(state, LUA_OK);
    lua_error(state);
    return 1;
}
int setVelocity(lua_State *state)
{
    int num_args = lua_gettop(state);
    if (num_args != 3)
    {
        std::string msg = "VELOCITY NOT SET! EXPECTED 3 ARGUMENTs BUT GOT " + num_args;
        spdlog::get("lua_logger")->error(msg);
        lua_pushinteger(state, LUA_ERRRUN);
        lua_error(state);
        return 1;
    }
    const char *entity_name = lua_tostring(state, 1);
    float vx = (float)lua_tonumber(state, 2);
    float vy = (float)lua_tonumber(state, 3);
    auto &world = GameWorld::getWorld();

    auto entity_id = world.getIdOf(entity_name);
    if (entity_id != -1)
    {
        auto p_entity = world.get(entity_id);
        assert(p_entity);
        p_entity->m_vel = {vx, vy};
    }
    lua_pushinteger(state, LUA_OK);
    lua_error(state);
    return 1;
}

//! \brief Registers functions with lua
void LuaWrapper::initializeLuaFunctions()
{
    lua_register(m_lua_state, "createObject", createObject);
    lua_register(m_lua_state, "createObjectAsChild", createObjectAsChild);
    lua_register(m_lua_state, "addEffect", addEffect);
    
    lua_register(m_lua_state, "setPosition", setPosition);
    lua_register(m_lua_state, "setTarget", setTarget);
    lua_register(m_lua_state, "setVelocity", setVelocity);

    lua_register(m_lua_state, "changeParentOf", changeParentOf);

    luabridge::getGlobalNamespace(m_lua_state)
        .beginNamespace("test")
        .addFunction("testA", createObject)
        .endNamespace();

    using namespace utils;

    luabridge::getGlobalNamespace(m_lua_state)
        .beginClass<utils::Vector2f>("Vec")
        .addConstructor<void(*) (float, float)>()
        .addProperty("x", &Vector2f::x)
        .addProperty("y", &Vector2f::y)
        .addFunction("__add", &Vector2f::operator+)
        .endClass();

    luabridge::getGlobalNamespace(m_lua_state)
        .beginClass<GameObject>("GameObject")
        .addProperty("x", &GameObject::getX, &GameObject::setX)
        .addProperty("y", &GameObject::getY, &GameObject::setY)
        .addProperty("vel", &GameObject::m_vel)
        .endClass()
        .deriveClass<Enemy, GameObject>("Enemy")
        .addProperty("state", &Enemy::getState, &Enemy::setState)
        .endClass()
        ;
    // .deriveClass<Enemy, GameObject>("Enemy")
    // .endClass()
}

//! \brief runs the command if it is registered
bool LuaWrapper::doString(const std::string &command)
{
    auto function_status = luaL_loadstring(m_lua_state, command.c_str());
    if (function_status == LUA_ERRSYNTAX)
    {
        spdlog::get("lua_logger")->error(" Syntax error during Precompilation of command: " + command + " !");
        return false;
    }
    if (function_status == LUA_ERRMEM)
    {
        spdlog::get("lua_logger")->error(" Memory out of bounds in command: " + command + " !");
        return false;
    }
    auto call_status = lua_pcall(m_lua_state, 0, LUA_MULTRET, 0);
    assert(lua_isinteger(m_lua_state, -1));
    auto error_status = lua_tointeger(m_lua_state, -1);
    if (error_status != LUA_OK)
    {
        return false;
    }
    return true;
}
//! \brief runs the command if it is registered
bool LuaWrapper::doFile(const std::string &filename)
{
    auto file_status = luaL_loadfile(m_lua_state, ("../scripts/" + filename).c_str());
    if (file_status == LUA_ERRFILE)
    {
        spdlog::get("lua_logger")->error(" could not load file: " + filename + " !");
        return false;
    }
    auto call_status = lua_pcall(m_lua_state, 0, LUA_MULTRET, 0);
    assert(lua_isinteger(m_lua_state, -1));
    auto error_status = lua_tointeger(m_lua_state, -1);
    if (error_status != LUA_OK)
    {
        return false;
    }
    return true;
}

std::string LuaWrapper::getLastError()
{
    if (m_ringbuffer_sink->last_formatted().size() > 0)
        return m_ringbuffer_sink->last_formatted().at(0);
    return "";
}

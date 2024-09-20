#include "LuaWrapper.h"
#include "GameWorld.h"
#include "AILuaComponent.h"
#include "Entities.h"
#include "Particles.h"

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <LuaBridge/LuaBridge.h>

//! Code for making the singleton work (Likely stolen)
LuaWrapper *LuaWrapper::s_instance = nullptr;
std::unordered_map<std::string, std::filesystem::file_time_type> LuaWrapper::m_script2last_change = {};

LuaWrapper *LuaWrapper::getSingleton()
{
    if (!s_instance)
    {
        s_instance = new LuaWrapper();
    }
    return s_instance;
}

bool LuaWrapper::loadScript(const std::string &script_name)
{

    std::filesystem::path path = "../scripts/" + script_name;
    auto num_chars = script_name.length();
    if (num_chars >= 4 && script_name.substr(num_chars - 4, num_chars) != ".lua") 
    {
        path += ".lua";
    }

    auto lua = getSingleton();
    auto last_change = std::filesystem::last_write_time(path);

    if (m_script2last_change.count(script_name) > 0 && m_script2last_change.at(script_name) == last_change)
    {
        return true; //! there was no change since last time in the script so we return
    }

    m_script2last_change[script_name] = last_change;

    //! load script
    auto script_load_status = luaL_dofile(lua->m_lua_state, path.c_str());

    if (script_load_status)
    {
        spdlog::get("lua_logger")->error("Script with name " + script_name + " not done");
        return false;
    }

     return true;
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
    lua_pushinteger(state, LUA_OK);
    lua_error(state);
    return 1;
}

static Projectile *createProjectile(lua_State *state)
{
    int num_args = lua_gettop(state);

    if (num_args != 2)
    {
        std::string msg = " OBJECT NOT CREATED! EXPECTED 2 ARGUMENTS BUT GOT " + std::to_string(num_args);
        spdlog::get("lua_logger")->error(msg);
        lua_pushinteger(state, LUA_ERRRUN);
        lua_error(state);
        return nullptr;
    }
    const char *object_type = lua_tostring(state, 1);
    const char *object_name = lua_tostring(state, 2);

    auto new_obj = GameWorld::getWorld().addObject("Bullet", object_name);

    return static_cast<Projectile *>(new_obj.get());
}

static Enemy *createEnemy(lua_State *state)
{
    int num_args = lua_gettop(state);

    if (num_args != 1)
    {
        std::string msg = " OBJECT NOT CREATED! EXPECTED 1 ARGUMENTS BUT GOT " + std::to_string(num_args);
        spdlog::get("lua_logger")->error(msg);
        lua_pushinteger(state, LUA_ERRRUN);
        lua_error(state);
        return nullptr;
    }
    const char *object_name = lua_tostring(state, 1);

    auto new_obj = GameWorld::getWorld().addObject("Enemy", object_name);

    return static_cast<Enemy *>(new_obj.get());
}

static GameObject *createObject2(lua_State *state)
{
    int num_args = lua_gettop(state);

    if (num_args != 2)
    {
        std::string msg = " OBJECT NOT CREATED! EXPECTED 2 ARGUMENTS BUT GOT " + std::to_string(num_args);
        spdlog::get("lua_logger")->error(msg);
        lua_pushinteger(state, LUA_ERRRUN);
        lua_error(state);
        return nullptr;
    }
    const char *object_type = lua_tostring(state, 1);
    const char *object_name = lua_tostring(state, 2);

    auto new_obj = GameWorld::getWorld().addObject(object_type, object_name);
    return new_obj.get();
}

static GameObject *getObject(lua_State *state)
{
    int num_args = lua_gettop(state);

    if (num_args != 1)
    {
        spdlog::get("lua_logger")->error("Object not fetched. EXPECTED 1 ARGUMENTS BUT GOT " + std::to_string(num_args));
        return nullptr;
    }
    const std::string object_name = lua_tostring(state, 1);

    auto new_obj = GameWorld::getWorld().get(object_name);
    if (new_obj)
    {
        return new_obj.get();
    }
    spdlog::get("lua_logger")->error("Object not fetched. Name " + object_name + " does not exist.");
    return nullptr;
}

static int removeObject(lua_State *state)
{
    int num_args = lua_gettop(state);

    if (num_args != 1)
    {
        std::string msg = " OBJECT NOT CREATED! EXPECTED 1 ARGUMENT BUT GOT " + std::to_string(num_args);
        spdlog::get("lua_logger")->error(msg);
        lua_pushinteger(state, LUA_ERRRUN);
        lua_error(state);
        return 1;
    }
    const char *object_name = lua_tostring(state, 1);

    auto &world = GameWorld::getWorld();
    auto id = world.getIdOf(object_name);
    if (id == -1)
    {
        std::string text = object_name;
        spdlog::get("lua_logger")->error("Object with name: " + text + " not found!");
        lua_pushinteger(state, LUA_ERRRUN);
        lua_error(state);
        return 1;
    }
    world.get(id)->kill();
    lua_pushinteger(state, LUA_OK);
    lua_error(state);
    return 1;
}

struct BookKeeper
{
};

static void removeObjects(lua_State *state)
{

    const std::string match_sequence = lua_tostring(state, 1);
    std::regex regex(match_sequence);

    auto &world = GameWorld::getWorld();
    for (auto &[name, id] : world.getNames())
    {
        if (std::regex_match(name.begin(), name.end(), regex))
        {
            assert(id != -1);
            world.get(id)->kill();
        }
    }
}

static int addEffect(lua_State *state)
{
    int num_args = lua_gettop(state);

    if (num_args != 2)
    {
        spdlog::get("lua_logger")->error(" OBJECT NOT CREATED! EXPECTED 2 ARGUMENTS BUT GOT " + std::to_string(num_args));
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

std::string getName(lua_State *state)
{
    int num_args = lua_gettop(state);
    if (num_args != 1)
    {
        std::string msg = "Wrong number of arguments. Expected 1 but got: " + num_args;
        spdlog::get("lua_logger")->error(msg);
    }
    int id = lua_tointeger(state, 1);
    auto name = GameWorld::getWorld().getName(id);
    return name;
};

//! \brief Registers functions with lua
void LuaWrapper::initializeLuaFunctions()
{
    lua_register(m_lua_state, "createObjectAsChild", createObjectAsChild);
    lua_register(m_lua_state, "addEffect", addEffect);

    lua_register(m_lua_state, "setPosition", setPosition);
    lua_register(m_lua_state, "setTarget", setTarget);
    lua_register(m_lua_state, "setVelocity", setVelocity);

    lua_register(m_lua_state, "changeParentOf", changeParentOf);

    using namespace utils;

    luabridge::getGlobalNamespace(m_lua_state)
        .addFunction("createObject", &createObject2)
        .addFunction("createProjectile", &createProjectile)
        .addFunction("createEnemy", &createEnemy)
        .addFunction("removeObject", &removeObject)
        .addFunction("removeObjects", &removeObjects)
        .addFunction("setPosition", &setPosition)
        .addFunction("setTarget", &setTarget)
        .addFunction("setVelocity", &setVelocity)
        .addFunction("getName", &getName)
        .addFunction("getObject", &getObject);

    luabridge::getGlobalNamespace(m_lua_state)
        .beginClass<utils::Vector2f>("Vec")
        .addConstructor<void (*)(float, float)>()
        .addProperty("x", &Vector2f::x)
        .addProperty("y", &Vector2f::y)
        .addFunction("__add", &Vector2f::operator+)
        // .addFunction("__sub", &Vector2f::oerator-)
        .endClass();

    luabridge::getGlobalNamespace(m_lua_state)
        .beginClass<Particle>("Particle")
        .addConstructor<void (*)(float, float)>()
        .addProperty("pos", &Particle::pos)
        .addProperty("vel", &Particle::vel)
        .addProperty("angle", &Particle::angle)
        .addProperty("scale", &Particle::scale)
        .addProperty("color", &Particle::color)
        .addProperty("life_time", &Particle::life_time)
        .addProperty("time", &Particle::time)
        .endClass();
    luabridge::getGlobalNamespace(m_lua_state)
        .beginClass<Particles>("Particles")
        .addConstructor<void (*)(int)>()
        .addProperty("spawn_pos", &Particles::m_spawn_pos)
        .addProperty("init_color", &Particles::m_init_color)
        .addProperty("final_color", &Particles::m_final_color)
        .addProperty("final_color", &Particles::m_final_color)
        .endClass();

    luabridge::getGlobalNamespace(m_lua_state)
        .beginClass<Color>("Color")
        .addConstructor<void (*)(float, float, float, float)>()
        .addProperty("r", &Color::r)
        .addProperty("g", &Color::g)
        .addProperty("b", &Color::b)
        .addProperty("a", &Color::a)
        .endClass();

    luabridge::getGlobalNamespace(m_lua_state)
        .beginClass<GameObject>("GameObject")
        .addProperty("x", &GameObject::getX, &GameObject::setX)
        .addProperty("y", &GameObject::getY, &GameObject::setY)
        .addProperty("target", &GameObject::getTarget, &GameObject::setTarget)
        .addProperty("target_pos", &GameObject::m_target_pos)
        .addProperty("id", &GameObject::getId)
        .addProperty("type", &GameObject::getType)
        .addProperty("vel", &GameObject::m_vel)
        .addFunction("kill", &GameObject::kill)
        .endClass()
        .deriveClass<Enemy, GameObject>("Enemy")
        .addProperty("state", &Enemy::getState, &Enemy::setState)
        .addProperty("script", &Enemy::getScript, &Enemy::setScript)
        .addProperty("health", &Enemy::m_health)
        .addProperty("type", &Enemy::m_health)
        .endClass()
        .deriveClass<Projectile, GameObject>("Projectile")
        .addProperty("max_vel", &Projectile::getMaxVel, &Projectile::setMaxVel)
        .addProperty("acc", &Projectile::getAcc, &Projectile::setAcc)
        .addProperty("owner", &Projectile::m_owner_entity_id)
        .addFunction("setScript", &Projectile::setScriptName)
        .endClass()
        .deriveClass<PlayerEntity, GameObject>("Player")
        .addProperty("max_vel", &PlayerEntity::getMaxSpeed, &PlayerEntity::setMaxSpeed)
        .addProperty("health", &PlayerEntity::m_health)
        .endClass()
        .deriveClass<Wall, GameObject>("Wall")
        .endClass();
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
    if (lua_isinteger(m_lua_state, -1)) //! if function returns it is an error and we return possibly
    {
        auto error_status = lua_tointeger(m_lua_state, -1);
        if (error_status != LUA_OK)
        {
            return false;
        }
    }
    return true;
}
//! \brief runs the script if it is exists
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

void reportErrors(lua_State *lua_state, int status)
{
    if (status == 0)
    {
        return;
    }

    std::string error_msg = lua_tostring(lua_state, -1);
    spdlog::get("lua_logger")->error("[LUA ERROR] " + error_msg);

    // remove error message from Lua state
    lua_pop(lua_state, 1);
}

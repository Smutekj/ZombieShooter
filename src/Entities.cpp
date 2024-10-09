#include "Entities.h"
#include "Wall.h"
#include "CollisionSystem.h"
#include "GameWorld.h"
#include "Utils/RandomTools.h"
#include "PathFinding/PathFinder.h"
#include "Map.h"

#include <Utils/Vector2.h>
#include <Particles.h>
#include <Font.h>

#ifdef __cplusplus
extern "C"
{
#include "../external/lua/src/lua.h"
#include "../external/lua/src/lualib.h"
#include "../external/lua/src/lauxlib.h"
};
#endif //__cplusplus

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "LuaWrapper.h"
#include <LuaBridge/LuaBridge.h>
#include <LuaBridge/Map.h>

#ifndef M_PIf
#define M_PIf std::numbers::pi_v<float>
#endif

namespace cdt
{
    template class Triangulation<utils::Vector2f>;
}

inline void truncate(utils::Vector2f &vec, float max_value)
{
    auto speed = norm(vec);
    if (speed > max_value)
    {
        vec *= max_value / speed;
    }
}
void drawAgent(utils::Vector2f pos, float radius, LayersHolder &layers, Color color, Color eye_color = {1, 0, 0, 1})
{
    radius /= std::sqrt(2.f);
    auto &canvas = layers.getCanvas("Unit");
    auto &eye_canvas = layers.getCanvas("Fire");

    float thickness = radius / 10.f;

    utils::Vector2f prev_pos = pos + utils::Vector2f{radius, 0};
    for (int i = 1; i < 51; ++i)
    {
        float x = pos.x + radius * std::cos(i / 50. * 2. * M_PIf);
        float y = pos.y + radius * std::sin(i / 50. * 2. * M_PIf);
        canvas.drawLineBatched(prev_pos, {x, y}, thickness, color, DrawType::Dynamic);
        prev_pos = {x, y};
    }
    utils::Vector2f left_eye_pos1 = pos + utils::Vector2f{-radius * 0.2, radius * 0.4};
    utils::Vector2f left_eye_pos2 = pos + utils::Vector2f{-radius * 0.6, radius * 0.5};
    utils::Vector2f right_eye_pos1 = pos + utils::Vector2f{+radius * 0.2, radius * 0.4};
    utils::Vector2f right_eye_pos2 = pos + utils::Vector2f{+radius * 0.6, radius * 0.5};

    // Sprite2 s();
    // canvas.drawCricleBatched(, 1.0, color, DrawType::Dynamic);
}

PlayerEntity::PlayerEntity(TextureHolder &textures)
    : GameObject(&GameWorld::getWorld(), textures, ObjectType::Player),
      m_vision(m_world->getTriangulation()),
      m_vision_verts(m_world->m_shaders.get("VisionLight2"))
{
    m_collision_shape = std::make_unique<Polygon>(4);
    setSize({20.f, 20.f});
}

void PlayerEntity::doScript()
{
}

void PlayerEntity::setMaxSpeed(float max_speed)
{
    m_max_speed = max_speed;
}

float PlayerEntity::getMaxSpeed() const
{
    return m_max_speed;
}

void PlayerEntity::update(float dt)
{
    utils::Vector2f m_look_dir = utils::approx_equal_zero(norm2(m_vel)) ? utils::dir2angle(m_angle) : m_vel / norm(m_vel);
    m_vision.constructField(cdt::Vector2f{m_pos.x, m_pos.y}, cdt::Vector2f{m_look_dir.x, m_look_dir.y});

    // if(m_disabilities.count())

    if (m_combat_state != CombatState::None)
    {
        m_vel *= 0.f;
    }

    truncate(m_vel, m_max_speed);
}

void PlayerEntity::onCreation()
{
}
void PlayerEntity::onDestruction() {}
void PlayerEntity::draw(LayersHolder &layers)
{
    try
    {
        auto lua = LuaWrapper::getSingleton();
        auto drawer = luabridge::getGlobal(lua->m_lua_state, "DrawPlayer");
        if (drawer.isFunction())
        {
            drawer(this, layers);
        }
    }
    catch (std::exception &e)
    {
        std::cout << "ERROR IN: " << e.what() << " in file player.lua" << "\n";
    }

    // drawAgent(m_pos, m_size.x, layers, {0, 0, 25, 1});

    auto &light_canvas = layers.getCanvas("Light");
    auto &shader = light_canvas.getShader("VisionLight");
    shader.use();
    shader.setUniform2("u_posx", m_pos.x);
    shader.setUniform2("u_posy", m_pos.y);
    shader.setUniform2("u_max_radius", m_vision_radius);
    m_vision.getDrawVertices(shader, m_vision_verts, {1, 1, 1, 1}, m_vision_radius);
    for (int vert_ind = 0; vert_ind < m_vision_verts.size(); ++vert_ind)
    {
        const auto &pos = m_vision_verts[vert_ind].pos;
        auto dr = pos - m_pos;
        m_vision_verts[vert_ind].tex_coord = {dr};     //! we use this information in a shader
        m_vision_verts[vert_ind].color = {1, 1, 1, 1}; //! we use this information in a shader
    }
    m_vision_verts.m_shader2.setUniform2("u_max_radius", m_vision_radius);
    light_canvas.drawVertices(m_vision_verts, DrawType::Dynamic);

    // light_canvas.drawCricleBatched(m_pos + m_vel*2.f, 50.f, Color{100,0,1,1.0});
}

void PlayerEntity::onCollisionWith(GameObject &obj, CollisionData &c_data)
{

    if (LuaWrapper::loadScript("collisions") != LuaScriptStatus::Ok)
    {
        return;
    }
    auto lua = LuaWrapper::getSingleton();

    switch (obj.getType())
    {
    case ObjectType::Bullet:
    {
        auto bullet = static_cast<Projectile *>(&obj);
        try
        {
            auto collider = luabridge::getGlobal(lua->m_lua_state, "PlayerBulletCollision");
            collider(this, &obj);
        }
        catch (std::exception &e)
        {
            std::cout << "ERROR IN PlayerBulletCollision: " << e.what() << "\n";
        }
        break;
    }
    case ObjectType::Player:
    {
        auto bullet = static_cast<PlayerEntity *>(&obj);
        try
        {
            auto collider = luabridge::getGlobal(lua->m_lua_state, "EnemyPlayerCollision");
            collider(this, &obj);
        }
        catch (std::exception &e)
        {
            std::cout << "ERROR IN EnemyPlayerCollision" << e.what() << "\n";
        }
        break;
    }
    case ObjectType::Wall:
    {
        auto wall = static_cast<Wall *>(&obj);
        try
        {

            auto collider = luabridge::getGlobal(lua->m_lua_state, "EnemyWallCollision");
            collider(this, &obj);
        }
        catch (std::exception &e)
        {
            std::cout << "ERROR IN EnemyWall" << e.what() << "\n";
        }
        break;
    }
    }
};

Event::Event(TextureHolder &textures, std::string event_script_name)
    : GameObject(&GameWorld::getWorld(), textures, ObjectType::Event),
      m_script_name(event_script_name)
{
    m_collision_shape = std::make_unique<Polygon>(8);
    setSize({20.f, 20.f});

    if (LuaWrapper::loadScript(event_script_name) != LuaScriptStatus::Ok)
    {
        return;
    }
}

void Event::update(float dt)
{
    if (LuaWrapper::loadScript(m_script_name) != LuaScriptStatus::Ok)
    {
        return;
    }

    try
    {
        auto update = luabridge::getGlobal(LuaWrapper::getSingleton()->m_lua_state, "EventUpdate");
        update(this);
    }
    catch (std::exception &e)
    {
        std::cout << "ERROR IN EventUpdate: " << e.what() << "\n";
    }
}

void Event::onCreation()
{
}
void Event::onDestruction()
{
}
void Event::draw(LayersHolder &layers)
{
}

void Event::onCollisionWith(GameObject &obj, CollisionData &c_data)
{

    if (LuaWrapper::loadScript(m_script_name) != LuaScriptStatus::Ok)
    {
        return;
    }
    auto lua = LuaWrapper::getSingleton();

    switch (obj.getType())
    {
    case ObjectType::Bullet:
    {
        auto bullet = static_cast<Projectile *>(&obj);
        try
        {
            auto collider = luabridge::getGlobal(lua->m_lua_state, "EventBulletCollision");
            collider(this, &obj);
        }
        catch (std::exception &e)
        {
            std::cout << "ERROR IN EventBulletCollision: " << e.what() << "\n";
        }
        break;
    }
    case ObjectType::Player:
    {
        auto player = static_cast<PlayerEntity *>(&obj);
        try
        {
            auto collider = luabridge::getGlobal(lua->m_lua_state, "EventPlayerCollision");
            collider(this, &obj);
        }
        catch (std::exception &e)
        {
            std::cout << "ERROR IN EventPlayerCollision" << e.what() << "\n";
        }
        break;
    }
    case ObjectType::Enemy:
    {
        auto enemy = static_cast<Enemy *>(&obj);
        try
        {
            auto collider = luabridge::getGlobal(lua->m_lua_state, "EventEnemyCollision");
            collider(this, &obj);
        }
        catch (std::exception &e)
        {
            std::cout << "ERROR IN EventEnemyCollision" << e.what() << "\n";
        }
        break;
    }
    case ObjectType::Wall:
    {
        auto wall = static_cast<Wall *>(&obj);
        try
        {

            auto collider = luabridge::getGlobal(lua->m_lua_state, "EventWallCollision");
            collider(this, &obj);
        }
        catch (std::exception &e)
        {
            std::cout << "ERROR IN EventWallCollision" << e.what() << "\n";
        }
        break;
    }
    }
};

// std::map<std::string, Color> getColorTable(std::string script_name,
//                                              std::unordered_map<std::string, std::shared_ptr<Particles>>& particles)
// {
//     // call function defined in Lua script
//     auto lua = LuaWrapper::getSingleton();
//     int script_load_status = luaL_dofile(lua->m_lua_state, "../scripts/firebolt.lua");
//     reportErrors(lua->m_lua_state, script_load_status);

//     auto colors_lua = luabridge::getGlobal(lua->m_lua_state, "ColorTable");
//     if(colors_lua.isNil())
//     {
//         return;
//     }

//     std::map<std::string, Color> colors;

//     auto& cl = colors_lua.cast<Projectile::ColorTable&>();

//     for(auto& [slot_name, color_map] : cl)
//     {
//         colors[slot_name] = {}
//     }

// }

void Projectile::readDataFromScript()
{
    auto lua = LuaWrapper::getSingleton();
    int script_load_status = luaL_dofile(lua->m_lua_state, ("../scripts/" + m_script_name).c_str());

    reportErrors(lua->m_lua_state, script_load_status);
    if (script_load_status)
    {
        spdlog::get("lua_logger")->error("Script could not loaded: " + m_script_name);
        return;
    }
    else
    {
        auto init_color = getVariable<Color>(lua->m_lua_state, "InitColor");
        auto final_color = getVariable<Color>(lua->m_lua_state, "FinalColor");
        m_bolt_particles.setInitColor(init_color);
        m_bolt_particles.setFinalColor(final_color);

        m_bolt_canvas_name = getVariable<std::string>(lua->m_lua_state, "BoltCanvas");
        m_tail_canvas_name = getVariable<std::string>(lua->m_lua_state, "TailCanvas");
        m_max_vel = getVariable<float>(lua->m_lua_state, "BoltSpeed");
    }

    auto spawner = luabridge::getGlobal(lua->m_lua_state, "Spawner");
    auto updater = luabridge::getGlobal(lua->m_lua_state, "Updater");

    if (spawner.isFunction())
    {
        m_bolt_particles.setEmitter(
            [this, spawner](auto pos)
            {
                Particle p;
                try
                {
                    p = spawner(pos, m_vel);
                }
                catch (std::exception &e)
                {
                    std::cout << "ERROR IN Spawner" << e.what() << " !";
                }
                return p;
            });
    }
    if (updater.isFunction())
    {
        m_bolt_particles.setUpdater(
            [this, updater](Particle &p)
            {
                try
                {
                    p = updater(p);
                }
                catch (std::exception &e)
                {
                    std::cout << "ERROR IN Spawner" << e.what() << " !";
                }
            });
    }
    // else
    {
        m_bolt_particles.setUpdater(
            [](Particle &p)
            {
                p.scale += utils::Vector2f{0.05f, 0.05f};
                p.pos += p.vel * 0.016f;
            });
    }

    m_bolt_particles.setLifetime(2.f);
    m_bolt_particles.setRepeat(true);

    auto tail_shader_id = getVariable<std::string>(lua->m_lua_state, "TailShader");
    m_shader_name = getVariable<std::string>(lua->m_lua_state, "BoltShader");
    // m_bolt_particles.setShader(tail_shader_id);
}

Projectile::Projectile(GameWorld *world, TextureHolder &textures, const std::string &script_name)
    : GameObject(world, textures, ObjectType::Bullet), m_bolt_particles(100), m_script_name(script_name)
{
    readDataFromScript();
    setTarget(utils::Vector2f{0, 0});
    m_collision_shape = std::make_unique<Polygon>(8);
    setSize({5.f, 5.f});
}

void Projectile::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    auto type = obj.getType();

    if (type == ObjectType::Enemy)
    {
        // auto &dmg_text = GameWorld::getWorld().addVisualEffect<FloatingText>("Text");
        // dmg_text.setPosition(m_pos + utils::Vector2f{0.f, 10.f});
        // dmg_text.m_lifetime = 5.f;
    }
};
void Projectile::onCreation()
{
}

std::shared_ptr<Font> m_font = nullptr;

void Projectile::onDestruction() {}
void Projectile::draw(LayersHolder &layers)
{
    if (!m_font)
    {
        m_font = std::make_shared<Font>("arial.ttf");
    }

    auto p_tail_canvas = layers.getCanvasP(m_tail_canvas_name);
    auto p_bolt_canvas = layers.getCanvasP(m_bolt_canvas_name);

    float angle = norm2(m_vel) > 0.f ? dir2angle(m_vel) : m_angle;
    setAngle(M_PIf / 180. * angle);

    m_bolt_particles.setSpawnPos(m_pos);
    m_bolt_particles.update(0.01f);
    if (p_tail_canvas)
    {
        m_bolt_particles.draw(*p_tail_canvas);
    }

    auto texture = m_textures->get("bomb");

    if (texture)
    {
        Sprite2 bolt_sprite(*texture);
        bolt_sprite.m_texture_handles[0] = 0; //.
        bolt_sprite.setPosition(m_pos);
        bolt_sprite.setRotation(m_angle);
        bolt_sprite.setScale(m_size);
        if (p_bolt_canvas)
        {
            p_bolt_canvas->drawSprite(bolt_sprite, m_shader_name, DrawType::Dynamic);
        }
    }
}

void Projectile::update(float dt)
{
    if (m_target)
    {
        if (m_target->isDead())
        {
            m_target = nullptr;
        }
        else
        {
            m_last_target_pos = m_target->getPosition();
        }
    }

    if (norm2(m_vel) > 0.0001)
    {
    }
    auto dr = m_last_target_pos - m_pos;
    auto dv = (dr / norm(dr) - m_vel / norm(m_vel));
    m_vel += dv * m_acc;
    m_vel *= m_max_vel / norm(m_vel);
    setPosition(m_pos);

    m_time += dt;
    if (m_time > m_lifetime)
    {
        kill();
    }
}

Enemy::Enemy(pathfinding::PathFinder &pf, TextureHolder &textures,
             Collisions::CollisionSystem &collider, PlayerEntity *player)
    : m_collision_system(&collider), m_pf(&pf), m_player(player), GameObject(&GameWorld::getWorld(), textures, ObjectType::Enemy)
{
    m_collision_shape = std::make_unique<Polygon>(4);
    setSize({10.f, 10.f});
    m_surfaces.toggleWalkable(SurfaceType::Ground);

    m_pathfinding_cd = (rand() % 15) + 60; //! randomize patfinding cd so that they do not do pathfind at the same frame
    m_path_rule = [&pf, this](cdt::TriInd i1, cdt::TriInd i2) -> bool
    {
        if (i2 == -1) //
        {
            return false;
        }

        auto surfaces = pf.m_surfaces;
        if (surfaces)
        {
            auto exit_surface = surfaces->m_tri2surface.at(i1).type;
            auto entry_surface = surfaces->m_tri2surface.at(i2).type;
            return m_surfaces.isWalkable(exit_surface);
        }
        return false;
    };
}

Enemy::~Enemy() {}

void Enemy::doScript()
{
    auto lua = LuaWrapper::getSingleton();

    if (LuaWrapper::loadScript("collisions.lua") == LuaScriptStatus::Broken)
    {
        return;
    }
    auto ai_script_status = LuaWrapper::loadScript("basicai.lua");
    if (ai_script_status == LuaScriptStatus::Broken)
    {
        return;
    }
    luabridge::LuaRef update_func = luabridge::getGlobal(lua->m_lua_state, "updateAI");
    if (update_func.isFunction())
    {
        m_ai_updater = [update_func, this]()
        {
            try
            {
                update_func(this);
            }
            catch (std::exception &e)
            {
                std::cout << "ERROR IN updateAI" << e.what() << " !\n";
            }
        };
    }
}

void Enemy::update(float dt)
{

    doScript();
    if (m_ai_updater)
    {
        m_ai_updater();
    }

    if (m_target)
    {
        if (m_target->isDead())
        {
            m_target = nullptr;
            m_target_pos = {-1, -1};
        }
        m_target_pos = m_target->getPosition();
    }

    if (m_pf)
    {
        if (m_pathfinding_timer++ >= m_pathfinding_cd)
        {
            m_pathfinding_timer = 0;
            if (m_state != AIState::Attacking)
            {
                // m_color = Color(0, 26, 1, 1);
                auto path_data = m_pf->doPathFinding({m_pos.x, m_pos.y}, {m_target_pos.x, m_target_pos.y}, m_size.x, m_path_rule);
                m_next_path = path_data.path.at(1);
                m_next_next_path = m_next_path;
                if (path_data.path.size() >= 3) //! if there is at least one extra point between start and end;
                {
                    m_next_next_path = path_data.path.at(2);
                }
            }
        }
    }

    boidSteering();

    truncate(m_acc, max_acc);
    m_vel += m_acc * dt;
    if (norm2(m_vel) > 0.000001)
    {
        m_angle = dir2angle(m_vel);
    }

    truncate(m_vel, max_vel);
    m_pos += (m_impulse)*dt;
    if (m_health < 0.f)
    {
        kill();
    }
    m_impulse *= 0.f;
    m_acc *= 0.f;
}

void Enemy::onCollisionWith(GameObject &obj, CollisionData &c_data)
{

    auto lua = LuaWrapper::getSingleton();

    switch (obj.getType())
    {
    case ObjectType::Enemy:
    {
        auto bullet = static_cast<Enemy *>(&obj);
        try
        {
            auto collider = luabridge::getGlobal(lua->m_lua_state, "EnemyEnemyCollision");
            collider(this, &obj);
        }
        catch (std::exception &e)
        {
            std::cout << "ERROR IN EnemyEnemyCollision: " << e.what() << "\n";
        }
        break;
    }
    case ObjectType::Bullet:
    {
        auto bullet = static_cast<Projectile *>(&obj);
        try
        {
            auto collider = luabridge::getGlobal(lua->m_lua_state, "EnemyBulletCollision");
            collider(this, &obj);
        }
        catch (std::exception &e)
        {
            std::cout << "ERROR IN EnemyBulletCollision: " << e.what() << "\n";
        }
        break;
    }
    case ObjectType::Player:
    {
        auto bullet = static_cast<PlayerEntity *>(&obj);
        try
        {
            auto collider = luabridge::getGlobal(lua->m_lua_state, "EnemyPlayerCollision");
            collider(this, &obj);
        }
        catch (std::exception &e)
        {
            std::cout << "ERROR IN EnemyPlayerCollision: " << e.what() << "\n";
        }
        break;
    }
    case ObjectType::Wall:
    {
        auto wall = static_cast<Wall *>(&obj);
        auto i1 = wall->m_cdt_edge.tri_ind;
        auto &triangles = m_pf->m_cdt.m_triangles;
        auto i2 = triangles.at(i1).neighbours[wall->m_cdt_edge.ind_in_tri];
        if (!m_path_rule(i1, i2) || wall->m_constraints_motion) //! constrain movement in we can't pathfind through the wall
        {
            auto &v = m_vel;
            auto pos = obj.getPosition();
            auto norm = wall->getNorm();
            auto n_dot_v = dot(v, norm);
            if (n_dot_v >= 0)
            {
                v -= n_dot_v * norm; //! remove normal compotent to the wall (How to make this work with many constraints?)
                m_color = Color{20, 1, 0, 1};
            }
        }

        try
        {
            auto collider = luabridge::getGlobal(lua->m_lua_state, "EnemyWallCollision");
            if (collider.isFunction())
            {
                collider(this, &obj);
            }
        }
        catch (std::exception &e)
        {
            std::cout << "ERROR IN EnemyWallCollision: " << e.what() << "\n";
        }
        break;
    }
    }
}

void Enemy::onCreation()
{
    if (LuaWrapper::loadScript("entitydied.lua") == LuaScriptStatus::Broken)
    {
        return;
    }
    auto lua = LuaWrapper::getSingleton();
    auto on_creation = luabridge::getGlobal(lua->m_lua_state, "OnEnemyCreation");
    if (on_creation.isFunction())
    {
        try
        {
            on_creation(this, std::string{"enemy1"});
        }
        catch (std::exception &e)
        {
            std::cout << "ERORR IN OnEnemyCreation: " << e.what() << " !\n";
        }
    }
}
void Enemy::onDestruction()
{
}

void Enemy::draw(LayersHolder &layers)
{
    try
    {
        auto lua = LuaWrapper::getSingleton();
        auto drawer = luabridge::getGlobal(lua->m_lua_state, "DrawEnemy");
        if (drawer.isFunction())
        {
            drawer(this, layers);
        }
    }
    catch (std::exception &e)
    {
        std::cout << "ERROR IN EnemyDraw: " << e.what() << "\n";
    }

    auto& canvas = layers.getCanvas("UI");
    canvas.drawLineBatched(m_pos, m_target_pos, 2., {1,0,0,1});
    canvas.drawLineBatched(m_pos, m_next_path, 2., {1,1,0,1});
    canvas.drawLineBatched(m_next_path, m_next_next_path, 2., {0,1,0,1});
}

void Enemy::avoidMeteors()
{
}

std::unordered_map<Multiplier, float> Enemy::m_force_multipliers = {
    {Multiplier::ALIGN, 0.f},
    {Multiplier::AVOID, 25000.f},
    {Multiplier::SCATTER, 10.f},
    {Multiplier::SEEK, 10.f}};
std::unordered_map<Multiplier, float> Enemy::m_force_ranges = {
    {Multiplier::ALIGN, 20.f},
    {Multiplier::AVOID, 30.f},
    {Multiplier::SCATTER, 3000.f},
    {Multiplier::SEEK, 10.f}};

void Enemy::boidSteering()
{
    // auto neighbours = m_neighbour_searcher->getNeighboursOfExcept(m_pos, m_boid_radius, m_id);

    auto neighbours = m_collision_system->findNearestObjects(ObjectType::Enemy, m_pos, m_boid_radius);

    utils::Vector2f repulsion_force(0, 0);
    utils::Vector2f push_force(0, 0);
    utils::Vector2f scatter_force(0, 0);
    utils::Vector2f cohesion_force(0, 0);
    utils::Vector2f seek_force(0, 0);
    float n_neighbours = 0;
    float n_neighbours_group = 0;
    utils::Vector2f dr_nearest_neighbours(0, 0);
    utils::Vector2f average_neighbour_position(0, 0);

    utils::Vector2f align_direction = {0, 0};
    int align_neighbours_count = 0;

    const float scatter_multiplier = Enemy::m_force_multipliers[Multiplier::SCATTER];
    const float align_multiplier = Enemy::m_force_multipliers[Multiplier::ALIGN];
    const float seek_multiplier = Enemy::m_force_multipliers[Multiplier::SEEK];

    auto range_align = std::pow(Enemy::m_force_ranges[Multiplier::ALIGN], 2);
    auto range_scatter = std::pow(Enemy::m_force_ranges[Multiplier::SCATTER], 2);

    using namespace utils;

    for (auto p_neighbour : neighbours)
    {
        if (p_neighbour == this)
        {
            continue;
        }
        auto &neighbour_boid = *p_neighbour;
        // if(ind_j == boid_ind){continue;}
        const auto dr = neighbour_boid.getPosition() - m_pos;
        const auto dist2 = norm2(dr);

        if (dist2 < range_align)
        {
            align_direction += neighbour_boid.m_vel;
            align_neighbours_count++;
        }

        if (dist2 < range_scatter)
        {
            scatter_force -= scatter_multiplier * dr / dist2;
            dr_nearest_neighbours += dr / dist2;
            n_neighbours++;
        }
        if (dist2 < range_scatter * 2.f)
        {
            average_neighbour_position += dr;
            n_neighbours_group++;
        }
    }

    dr_nearest_neighbours /= n_neighbours;

    if (n_neighbours > 0 && !utils::approx_equal_zero(norm2(dr_nearest_neighbours)))
    {
        scatter_force += -scatter_multiplier * dr_nearest_neighbours / norm(dr_nearest_neighbours) - m_vel;
    }

    average_neighbour_position /= n_neighbours_group;
    if (n_neighbours_group > 0)
    {
        // cohesion_force =   * average_neighbour_position - m_vel;
    }

    utils::Vector2f align_force = {0, 0};
    if (align_neighbours_count > 0 && norm2(align_direction) >= 0.001f)
    {
        align_force = align_multiplier * align_direction / norm(align_direction) - m_vel;
    }

    if (m_state != AIState::Attacking)
    {
        auto dr_to_target = m_next_path - m_pos;
        if (norm(dr_to_target) > 3.f)
        {
            seek_force = seek_multiplier * max_vel * dr_to_target / norm(dr_to_target) - m_vel;
        }
        else
        {
            if (m_next_path == m_next_next_path && m_target) //! we reached destination or we need another path
            {
                if (utils::dist(m_next_path, m_target_pos) < 2.f) //! if we approach destination
                {
                    m_vel *=0.5f;
                }
                else
                {
                    m_pathfinding_timer = m_pathfinding_cd; //! force pathfinding
                }
            }
            else
            {
                m_next_path = m_next_next_path;
            }
        }
    }

    m_acc += (scatter_force + align_force + seek_force + cohesion_force);
    truncate(m_acc, max_acc);
}

OrbitingShield::OrbitingShield(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::Orbiter)
{
    m_particles = std::make_unique<Particles>(100);
    readDataFromScript();
    setSize({50., 50.});
}

void OrbitingShield::update(float dt)
{
    m_time += dt;
    float orbit_angle = 180. * m_time * m_orbit_speed;
    m_transform.setPosition(0.f * utils::angle2dir(orbit_angle));
    if (m_time > m_lifetime)
    {
        kill();
    }
}

void OrbitingShield::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

void OrbitingShield::onCreation()
{
}
void OrbitingShield::onDestruction()
{
}

void OrbitingShield::draw(LayersHolder &layers)
{
    auto &unit_canvas = layers.getCanvas("Unit");
    auto &particle_canvas = layers.getCanvas("Wall");

    Sprite2 rect(*m_textures->get("bomb"));
    rect.m_texture_handles[0] = 0;
    rect.setPosition(m_pos);
    rect.setRotation(dir2angle(m_vel));
    rect.setScale(m_size);

    unit_canvas.drawSprite(rect, m_shader_name, DrawType::Dynamic);

    m_particles->setSpawnPos(m_pos);
    m_particles->update(0.01f);
    m_particles->draw(particle_canvas);
}

Shield::Shield(TextureHolder &textures)
    : GameObject(&GameWorld::getWorld(), textures, ObjectType::Orbiter)
{
}

void Shield::update(float dt)
{
    m_time += dt;
    float orbit_angle = 180. * m_time * m_orbit_speed;
    m_transform.setPosition(0.f, 0.f);
    if (m_time > m_lifetime)
    {
        kill();
    }
}

void Shield::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

void Shield::onCreation()
{
}
void Shield::onDestruction()
{
}

void Shield::draw(LayersHolder &layers)
{
    auto &unit_canvas = layers.getCanvas("Unit");

    Sprite2 rect(*m_textures->get("bomb"));
    rect.m_texture_handles[0] = 0;
    rect.setPosition(m_pos);
    rect.setRotation(utils::dir2angle(m_vel));
    rect.setScale(m_size * 2.f);

    unit_canvas.drawSprite(rect, "basicshield", DrawType::Dynamic);
}

void OrbitingShield::readDataFromScript()
{

    auto lua = LuaWrapper::getSingleton();
    int script_load_status = luaL_dofile(lua->m_lua_state, ("../scripts/" + m_script_name).c_str());

    reportErrors(lua->m_lua_state, script_load_status);
    if (script_load_status)
    {
        spdlog::get("lua_logger")->error("Script could not loaded: " + m_script_name);
        return;
    }
    else
    {
        // auto particles_script = luabridge::getGlobal(lua->m_lua_state, "InitParticles");
        // if(particles_script.isFunction())
        // {
        //     try
        //     {
        //         particles_script(m_particles);
        //     }
        //     catch(const std::exception& e)
        //     {
        //         std::cerr << e.what() << '\n';
        //     }

        // }
        auto init_color = getVariable<Color>(lua->m_lua_state, "InitColor");
        auto final_color = getVariable<Color>(lua->m_lua_state, "FinalColor");
        auto spawn_period = getVariable<int>(lua->m_lua_state, "SpawnPeriod");
        m_particles->setInitColor(init_color);
        m_particles->setFinalColor(final_color);
        m_particles->setPeriod(spawn_period);
    }

    auto spawner = luabridge::getGlobal(lua->m_lua_state, "Spawner");
    auto updater = luabridge::getGlobal(lua->m_lua_state, "Updater");

    if (spawner.isFunction())
    {
        m_particles->setEmitter(
            [this, spawner](auto pos)
            {
                Particle p;
                try
                {
                    auto &world = GameWorld::getWorld();
                    auto parent_id = world.m_scene.getNode(getId()).parent;
                    auto owner = world.get(parent_id);
                    auto vel = m_vel;
                    if (owner)
                    {
                        vel = owner->m_vel;
                    }
                    p = spawner(pos, vel);
                }
                catch (std::exception &e)
                {
                    std::cout << "ERROR IN Spawner: " << e.what() << " !";
                }
                return p;
            });
    }
    if (updater.isFunction())
    {
        m_particles->setUpdater(
            [this, updater](Particle &p)
            {
                try
                {
                    p = updater(p);
                }
                catch (std::exception &e)
                {
                    std::cout << "ERROR IN Updater: " << e.what() << " !";
                }
            });
    }
    else
    {
        m_particles->setUpdater(
            [](Particle &p)
            {
                p.pos += p.vel;
            });
    }

    m_particles->setRepeat(true);

    auto tail_shader_id = getVariable<std::string>(lua->m_lua_state, "TailShader");
    auto shader_name = getVariable<std::string>(lua->m_lua_state, "BoltShader");
    if (shader_name != "")
    {
        m_shader_name = shader_name;
    }
    m_particles->setShader(tail_shader_id);
}
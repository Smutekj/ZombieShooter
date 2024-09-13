#include "Entities.h"
#include "CollisionSystem.h"
#include "GameWorld.h"
#include "Utils/RandomTools.h"
#include "PathFinding/PathFinder.h"

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

#ifndef M_PIf
#define M_PIf std::numbers::pi_v<float>
#endif

namespace cdt
{
    template class Triangulation<utils::Vector2f>;
}

static float dir2angle(utils::Vector2f dir)
{
    return 180.f * std::atan2(dir.y, dir.x) / M_PIf;
}

void drawAgent(utils::Vector2f pos, float radius, LayersHolder &layers, Color color)
{
    auto &canvas = layers.getCanvas("Unit");

    float thickness = radius / 10.f;

    utils::Vector2f prev_pos = pos + utils::Vector2f{radius, 0};
    for (int i = 1; i < 51; ++i)
    {
        float x = pos.x + radius * std::cos(i / 50. * 2. * M_PIf);
        float y = pos.y + radius * std::sin(i / 50. * 2. * M_PIf);
        canvas.drawLineBatched(prev_pos, {x, y}, thickness, color, GL_DYNAMIC_DRAW);
        prev_pos = {x, y};
    }
    utils::Vector2f left_eye_pos1 = pos + utils::Vector2f{-radius * 0.2, radius * 0.4};
    utils::Vector2f left_eye_pos2 = pos + utils::Vector2f{-radius * 0.6, radius * 0.5};
    utils::Vector2f right_eye_pos1 = pos + utils::Vector2f{+radius * 0.2, radius * 0.4};
    utils::Vector2f right_eye_pos2 = pos + utils::Vector2f{+radius * 0.6, radius * 0.5};
    canvas.drawLineBatched(left_eye_pos1, left_eye_pos2, thickness / 2.f, color, GL_DYNAMIC_DRAW);
    canvas.drawLineBatched(right_eye_pos1, right_eye_pos2, thickness / 2.f, color, GL_DYNAMIC_DRAW);
    // canvas.drawCricleBatched(, 1.0, color, GL_DYNAMIC_DRAW);
}

PlayerEntity::PlayerEntity(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::Player),
      m_vision(world->getTriangulation()),
      m_vision_verts(world->m_shaders.get("VisionLight"))
{
    m_collision_shape = std::make_unique<Polygon>(4);
    setSize({6.f, 6.f});
}

void PlayerEntity::update(float dt)
{
    utils::Vector2f m_look_dir = utils::approx_equal_zero(norm2(m_vel)) ? dir2angle(m_angle) : m_vel / norm(m_vel);
    m_vision.contrstuctField(cdt::Vector2f{m_pos.x, m_pos.y}, cdt::Vector2f{m_look_dir.x, m_look_dir.y});
}
void PlayerEntity::onCreation()
{
}
void PlayerEntity::onDestruction() {}
void PlayerEntity::draw(LayersHolder &layers)
{

    drawAgent(m_pos, m_size.x * 2.f, layers, {0, 0, 25, 1});

    auto &light_canvas = layers.getCanvas("Light");
    auto &shader = light_canvas.getShader("VisionLight");
    shader.use();
    shader.setUniform2("u_posx", m_pos.x);
    shader.setUniform2("u_posy", m_pos.y);
    m_vision.getDrawVertices(shader, m_vision_verts);
    for (int vert_ind = 0; vert_ind < m_vision_verts.size(); ++vert_ind)
    {
        const auto &pos = m_vision_verts[vert_ind].pos;
        auto dr = pos - m_pos;
        m_vision_verts[vert_ind].tex_coord = {dr}; //! we use this information in a shader
    }
    light_canvas.drawVertices(m_vision_verts, GL_DYNAMIC_DRAW);

    // light_canvas.drawCricleBatched(m_pos + m_vel*2.f, 50.f, Color{100,0,1,1.0});
}

void PlayerEntity::onCollisionWith(GameObject &obj, CollisionData &c_data) {

};

Wall::Wall(TextureHolder &textures, utils::Vector2f from, utils::Vector2f to)
    : GameObject(&GameWorld::getWorld(), textures, ObjectType::Wall)
{
    m_collision_shape = std::make_unique<Polygon>(2);
    m_collision_shape->points.at(0) = {-0.5f, 0.f};
    m_collision_shape->points.at(1) = {0.5f, 0.f};
    // m_collision_shape->points.at(2) = {-0.5f, 0.f};
    auto dr = from - to;
    auto angle = dir2angle(dr);
    auto length = dist(from, to);
    setPosition((from + to) / 2.f);
    setAngle(angle);
    setSize({length, 1.f});
}

void Wall::update(float dt)
{
}
void Wall::onCreation()
{
}
void Wall::onDestruction() {}
void Wall::draw(LayersHolder &layers)
{
    auto &canvas = layers.getCanvas("Wall");

    auto points = m_collision_shape->getPointsInWorld();
    assert(points.size() >= 2);
    auto v0 = points.at(0);
    auto v1 = points.at(1);
    canvas.drawLineBatched(v1, v0, 1.0, m_color);

    // auto norm = getNorm();
    // canvas.drawLineBatched(getPosition(), getPosition() + 5.f*norm, 1., {1,0,0,1});
}
void Wall::onCollisionWith(GameObject &obj, CollisionData &c_data)
{

    if (obj.getType() == ObjectType::Player || obj.getType() == ObjectType::Enemy)
    {
        auto &v = obj.m_vel;
        auto pos = obj.getPosition();
        auto norm = getNorm();
        auto n_dot_v = dot(v, norm);
        if (n_dot_v >= 0)
        {
            v -= n_dot_v * norm;
            m_color = Color{20, 1, 0, 1};
        }
    }
    if (obj.getType() == ObjectType::Bullet)
    {
        obj.kill();
    }
};

Projectile::Projectile(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::Bullet), m_bolt_particles(200)
{
    m_bolt_particles.setInitColor({0., 0.5, 20., 1.});
    m_bolt_particles.setFinalColor({1., 0.6, 2., 0.2});
    m_bolt_particles.setEmitter(
        [this](auto pos)
        {
            Particle p;
            p.pos = pos + m_size.x / 2.f * utils::Vector2f{std::cos(randf(2 * M_PIf)), std::sin(randf(2 * M_PIf))} - m_vel / norm(m_vel) * m_size.x / 2.f;
            p.vel = {-m_vel.y, m_vel.x};
            p.vel *= (2. * randf() - 1.) / norm(m_vel) / 33.69f;
            return p;
        });
    m_bolt_particles.setUpdater(
        [](Particle &p)
        {
            p.scale = {2.f, 2.f};
            p.pos += p.vel;
        });
    m_bolt_particles.setLifetime(2.f);
    m_bolt_particles.setRepeat(true);
    setTarget(utils::Vector2f{0, 0});

    m_collision_shape = std::make_unique<Polygon>(8);
    setSize({5.f, 5.f});
}

void Projectile::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
    auto type = obj.getType();
    if (type == ObjectType::Enemy)
    {
        static_cast<Enemy &>(obj).m_health--;
    }
};
void Projectile::onCreation()
{
}

std::shared_ptr<Font> m_font = nullptr;

void Projectile::onDestruction() {}
void Projectile::draw(LayersHolder &layers)
{

    int number = 0;

    if (!m_font)
    {
        m_font = std::make_shared<Font>("arial.ttf");
    }

    auto &canvas = layers.getCanvas("Unit");

    Text name("Test");
    name.setFont(m_font);
    name.setPosition(m_pos + utils::Vector2f{0.f, m_size.y * 2.f});
    canvas.drawText(name, "Instanced", GL_DYNAMIC_DRAW);

    float angle = norm2(m_vel) > 0.f ? dir2angle(m_vel) : m_angle;
    setAngle(angle);

    m_bolt_particles.setSpawnPos(m_pos);
    m_bolt_particles.update(0.01f);
    m_bolt_particles.draw(canvas);
    canvas.drawCricleBatched(m_pos, m_angle, 1.1f * m_size.x, 0.9f * m_size.y, {0, 2.0, 10., 0.5});
}

void Projectile::update(float dt)
{
    std::visit(
        [this](auto &target)
        {
        using T = uncvref_t<decltype(target)>;
        if constexpr (std::is_same_v<T, GameObject *>)
        {
            if (target)
            {

                if (target->isDead())
                {
                    m_target = nullptr;
                }else{
                    m_last_target_pos = target->getPosition();
                }
            }
            else if constexpr (std::is_same_v<T, utils::Vector2f>)
            {
            }

        } },
        m_target);

    if (norm2(m_vel) > 0.0001)
    {
    }
    auto dr = m_last_target_pos - m_pos;
    auto dv = (dr / norm(dr) - m_vel / norm(m_vel));
    m_vel += dv * 0.5f;
    m_vel *= 50. / norm(m_vel);
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
    // m_target_pos = player->getPosition();
}

Enemy::~Enemy() {}

inline void truncate(utils::Vector2f &vec, float max_value)
{
    auto speed = norm(vec);
    if (speed > max_value)
    {
        vec *= max_value / speed;
    }
}

void Enemy::update(float dt)
{

    auto lua = LuaWrapper::getSingleton();
    int scriptLoadStatus = luaL_dofile(lua->m_lua_state, "../scripts/basicai.lua");
    luabridge::LuaRef processFunc = luabridge::getGlobal(lua->m_lua_state, "UpdateAI");
    if (processFunc.isFunction())
    {
        try
        {
            processFunc(static_cast<GameObject *>(this));
        }
        catch (std::exception e)
        {
            std::cout << e.what() << " !\n";
        }
    }

    if (m_target)
    {
        if (m_target->isDead())
        {
            m_target = nullptr;
            m_target_pos = {-1, -1};
        }
        m_target_pos = m_target->getPosition();

        auto dist2target = dist(m_target_pos, m_pos);
        if (dist2target < 500 && m_state != AIState::Attacking)
        {

            m_state = AIState::Attacking;
        }
        else
        {
            m_state = AIState::Chasing;
        }
    }
    else
    {
        m_state = AIState::Patroling;
    }

    if (m_pf && m_target_pos.x > 0)
    {
        if (utils::norm2(m_target_pos - m_pos) <= 500 * 500 && m_state == AIState::Chasing)
        {
            // m_color = Color(0, 26, 1, 1);
            auto path_data = m_pf->doPathFinding({m_pos.x, m_pos.y}, {m_target_pos.x, m_target_pos.y}, m_size.x);
            m_next_path = path_data.path.at(1);
        }
        else
        {
            // m_color = Color(20, 0, 0, 1);
        }
    }

    boidSteering();
    // avoidMeteors();

    truncate(m_acc, max_acc);
    m_vel += m_acc * dt;

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
    switch (obj.getType())
    {
    case ObjectType::Bullet:
    {
        auto &bullet = static_cast<Projectile &>(obj);
        m_health--;
        break;
    }
    }
}

void Enemy::onCreation()
{
}
void Enemy::onDestruction()
{
}

void Enemy::draw(LayersHolder &layers)
{
    drawAgent(m_pos, m_size.x, layers, m_color);

    // for(auto pos : m_cm)
    // {
    //     auto dr = pos - m_pos;

    //     line.setPosition(m_pos);
    //     line.setRotation(dir2angle(dr));
    //     line.setSize({norm(dr), 1.f});
    //     line.setOrigin({0, 0.5f });
    //     target.draw(line);
    // }
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
    {Multiplier::SCATTER, 30.f},
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
    }

    m_acc += (scatter_force + align_force + seek_force + cohesion_force);
    truncate(m_acc, max_acc);
}

OrbitingShield::OrbitingShield(GameWorld *world, TextureHolder &textures)
    : GameObject(world, textures, ObjectType::Orbiter)
{
}

void OrbitingShield::update(float dt)
{
    m_time += dt;
    float orbit_angle = 180. * m_time * m_orbit_speed;
    m_transform.setPosition(50.f * utils::angle2dir(orbit_angle));
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

    Sprite2 rect(*m_textures->get("bomb"));
    rect.m_texture_handles[0] = 0;
    rect.setPosition(m_pos);
    rect.setRotation(dir2angle(m_vel));
    rect.setScale(m_size * 2.f);

    unit_canvas.drawSprite(rect, "lightning", GL_DYNAMIC_DRAW);
}
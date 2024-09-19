#pragma once

#include "GameObject.h"
#include <Particles.h>

#include "AILuaComponent.h"
#include "Shadows/VisibilityField.h"

class PlayerEntity : public GameObject
{

public:
    PlayerEntity() = default;
    PlayerEntity(GameWorld *world, TextureHolder &textures);
    virtual ~PlayerEntity() override {}

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    VisionField m_vision;
    VertexArray m_vision_verts;
};

class Wall : public GameObject
{

public:
    Wall() = default;
    explicit Wall(TextureHolder &textures, utils::Vector2f from, utils::Vector2f to);
    virtual ~Wall() override {}

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    utils::Vector2f getNorm()
    {

        auto points = m_collision_shape->getPointsInWorld();
        assert(points.size() >= 2);
        auto v0 = points.at(0);
        auto v1 = points.at(1);
        auto t = (v1 - v0);
        t /= norm(t);
        m_norm = utils::Vector2f{t.y, -t.x};
        return m_norm;
    }

private:
    utils::Vector2f m_norm = {0, 0};

private:
    Color m_color = {0, 1.f, 0, 1.f};
};

using ProjectileTarget = std::variant<GameObject *, utils::Vector2f>;
using ProjectileCaster = std::variant<GameObject *, PlayerEntity *>;

namespace Collisions
{
    class CollisionSystem;
}

namespace pathfinding
{
    class PathFinder;
}

class Enemy : public GameObject
{

public:
    Enemy() = default;
    Enemy(pathfinding::PathFinder &pf, TextureHolder &textures, Collisions::CollisionSystem &collider, PlayerEntity *player);
    virtual ~Enemy() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

public:
    int getState() const
    {
        return static_cast<int>(m_state);
    }
    void setState(int state)
    {
        if (state >= 3)
        {
            return;
        }
        m_state = static_cast<AIState>(state);
    }

    const std::string &getScript() const
    {
        return m_script_name;
    }

    void setScript(const std::string script_name)
    {
        m_script_name = script_name;
    }

private:
    void avoidMeteors();
    void boidSteering();
    void doScript();

public:
    float max_vel = 40.f;
    const float max_acc = 200.f;
    const float max_impulse_vel = 40.f;

    float m_health = 5;
    utils::Vector2f m_impulse = {0, 0};

    utils::Vector2f m_next_path = {-1, -1};
    AIState m_state = AIState::Patroling;

    Color m_color = Color(20, 0, 0, 1);

public:
    static std::unordered_map<Multiplier, float> m_force_multipliers;
    static std::unordered_map<Multiplier, float> m_force_ranges;

private:
    float m_boid_radius = 30.f;
    utils::Vector2f m_acc = {0, 0};

    Collisions::CollisionSystem *m_collision_system = nullptr;
    pathfinding::PathFinder *m_pf = nullptr;
    PlayerEntity *m_player = nullptr;

    bool m_is_avoiding = false;

    std::string m_script_name = "basicai.lua";
};

template <class T>
using uncvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
template <typename T>
struct fail : std::false_type
{
};

class Projectile : public GameObject
{

public:
    using ColorTable = std::map<std::string, std::map<std::string, float>>;

public:
    Projectile() = default;
    Projectile(GameWorld *world, TextureHolder &textures, const std::string &script_name = "firebolt.lua");
    virtual ~Projectile() override {}

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    void setScriptName(const std::string &new_script_name)
    {
        m_script_name = new_script_name;
        readDataFromScript();
    }

    const std::string&  getScriptName() const
    {
        return m_script_name;
    }

    void setMaxVel(float new_vel)
    {
        m_max_vel = new_vel;
    }

    float getMaxVel() const
    {
        return m_max_vel;
    }

    void setAcc(float new_acc)
    {
        m_acc = new_acc;
    }

    float getAcc() const
    {
        return m_acc;
    }

    void setTarget(ProjectileTarget target)
    {
        std::visit(
            [this](auto &target)
            {
                using T = uncvref_t<decltype(target)>;
                if constexpr (std::is_same_v<T, GameObject *>)
                {
                    if (target)
                    {
                        m_last_target_pos = target->getPosition();
                    }
                }
                else if constexpr (std::is_same_v<T, utils::Vector2f>)
                {
                    m_last_target_pos = target;
                }
            },
            target);
        auto dr = m_last_target_pos - m_pos;
        m_vel = (dr) / norm(dr) * m_max_vel;
    }

    int m_owner_entity_id = -1;
private:
    void readDataFromScript();
protected:
    // ProjectileTarget m_target = nullptr;
    utils::Vector2f m_last_target_pos = {0, 0};
    float m_max_vel = 50.f;
    float m_acc = 20.f;

    Particles m_bolt_particles;
    std::unordered_map<std::string, std::shared_ptr<Particles>> m_particles;
    float m_time = 0.f;
    float m_lifetime = 10.f;

    std::string m_shader_name = "fireBolt";
    std::string m_script_name = "firebolt";
    std::string m_bolt_canvas_name = "Wall";
    std::string m_tail_canvas_name = "Wall";
};

struct Timer
{
    float m_lifetime = 0.f;
    float m_time = 0.f;
};

class OrbitingShield : public GameObject
{
public:
    OrbitingShield() = default;
    OrbitingShield(GameWorld *world, TextureHolder &textures);
    virtual ~OrbitingShield() override {}

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    void readDataFromScript();

public:
    std::string m_script_name = "basicshield.lua";
    std::string m_shader_name = "basicshield";

private:
    float m_orbit_speed = 2.f;
    float m_time = 0.f;
    float m_lifetime = 500000.f;

    std::unique_ptr<Particles> m_particles;

    
};
class Shield : public GameObject
{
public:
    Shield() = default;
    Shield(TextureHolder &textures);
    virtual ~Shield() override {}

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    float m_orbit_speed = 2.f;
    float m_time = 0.f;
    float m_lifetime = 500000.f;

    std::unique_ptr<Particles> m_particles;
};

struct ProjectileData
{
    GameObject *m_caster = nullptr;
    GameObject *m_target = nullptr;
};

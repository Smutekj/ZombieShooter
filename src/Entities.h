#pragma once

#include "GameObject.h"
#include <Particles.h>

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
};

using ProjectileTarget = std::variant<GameObject *, utils::Vector2f>;
using ProjectileCaster = std::variant<GameObject *, PlayerEntity *>;

namespace Collisions
{
    class CollisionSystem;
}

class Enemy : public GameObject
{

public:
    Enemy() = default;
    Enemy(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem &collider, PlayerEntity *player);
    virtual ~Enemy() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

public:
    float max_vel = 40.f;
    const float max_acc = 200.f;
    const float max_impulse_vel = 40.f;

    float m_health = 5;
    utils::Vector2f m_impulse = {0, 0};
    utils::Vector2f m_target_pos;
    std::vector<utils::Vector2f> m_cm;

private:
    void avoidMeteors();
    void boidSteering();

public:
    static std::unordered_map<Multiplier, float> m_force_multipliers;
    static std::unordered_map<Multiplier, float> m_force_ranges;

private:
    float m_boid_radius = 30.f;
    utils::Vector2f m_acc;

    PlayerEntity *m_player;
    Collisions::CollisionSystem *m_collision_system;

    bool m_is_avoiding = false;
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
    Projectile() = default;
    Projectile(GameWorld *world, TextureHolder &textures);
    virtual ~Projectile() override {}

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    void setTarget(ProjectileTarget target)
    {
        m_target = target;
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
            m_target);
        auto dr = m_last_target_pos - m_pos;
        m_vel = (dr) / norm(dr) * 50.f;
    }

protected:
    ProjectileTarget m_target = nullptr;
    utils::Vector2f m_last_target_pos = {0, 0};
    Particles m_bolt_particles;

    float m_time = 0.f;
    float m_lifetime = 10.f;
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
    float m_orbit_speed = 2.f;
    float m_time = 0.f;
    float m_lifetime = 5.f;

};

struct ProjectileData
{
    GameObject *m_caster = nullptr;
    GameObject *m_target = nullptr;
};

#pragma once

#include "GameObject.h"
#include <Particles.h>

class PlayerEntity : public GameObject
{

public:
    PlayerEntity(GameWorld *world, TextureHolder &textures, ObjectType type);
    virtual ~PlayerEntity() override {}

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;
};

using ProjectileTarget = std::variant<GameObject *, utils::Vector2f>;
using ProjectileCaster = std::variant<GameObject *, PlayerEntity *>;

class Enemy : public GameObject
{
    Enemy(GameWorld *world, TextureHolder &textures, ObjectType type);
    virtual ~Enemy() override {}

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;
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
    Projectile(GameWorld *world, TextureHolder &textures, ObjectType type);
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

struct ProjectileData
{
    GameObject *m_caster = nullptr;
    GameObject *m_target = nullptr;
};

class ProjectilePhysics
{

    std::vector<Projectile *> m_projectiles;
};
#pragma once

#include <memory>
#include <functional>

#include <Vector2.h>
#include <Renderer.h>

#include "Polygon.h"
#include "DrawLayer.h"

class GameWorld;

enum class Multiplier
{
    SCATTER,
    ALIGN,
    SEEK,
    VELOCITY,
    AVOID
};

struct CollisionData
{
    utils::Vector2f separation_axis;
    float minimum_translation = -1;
    bool belongs_to_a = true;
    utils::Vector2f contact_point;
};

enum class ObjectType
{
    Enemy,
    Bullet,
    Player,
    Wall,
    Orbiter,
    VisualEffect,
    Count
};

enum class EffectType
{
    Fire = static_cast<int>(ObjectType::Count),
    Water,
    ParticleEmiter,
    AnimatedSprite,

};

enum class ObjectiveType
{
    ReachSpot,
    DestroyEnemy,
    Count
};

struct RigidBody
{
    float mass;
    float inertia;
    float angle_vel;
};

class GameObject
{

public:
    GameObject(GameObject& left)
    : m_textures(left.m_textures)
    {
        m_collision_shape = std::move(left.m_collision_shape);
    }
    GameObject() = default;
    GameObject(GameWorld *world, TextureHolder &textures, ObjectType type);

    virtual void update(float dt) = 0;
    virtual void onCreation() = 0;
    virtual void onDestruction() {m_on_destruction_callback(m_id, m_type);}
    virtual void draw(LayersHolder &layers) = 0;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) = 0;
    virtual ~GameObject() {}

    void removeCollider();
    void kill();
    bool isDead() const;
    void updateAll(float dt);

    const utils::Vector2f &getPosition() const;
    void setX(float x) ;
    void setY(float y) ;
    float getX() const;
    float getY() const;
    void setTarget(GameObject* target);
    GameObject* getTarget()const;
    void setPosition(utils::Vector2f new_position);
    void move(utils::Vector2f by);

    float getAngle() const;
    void setAngle(float angle);
    
    bool collides() const;
    Polygon &getCollisionShape();
    
    bool doesPhysics()const;
    RigidBody &getRigidBody();
    
    int getId() const;
    const ObjectType& getType() const;
    
    void setSize(utils::Vector2f size);

public:

    utils::Vector2f m_pos;
    utils::Vector2f m_target_pos;
    GameObject* m_target = nullptr;
    
    int m_id;
    Transform m_transform;
    utils::Vector2f m_vel = {0, 0};
    std::string m_name = "DefaultName";

protected:
    TextureHolder* m_textures;
    GameWorld *m_world;
    
    std::unique_ptr<Polygon> m_collision_shape = nullptr;
    std::unique_ptr<RigidBody> m_rigid_body = nullptr;
    
    utils::Vector2f m_size = {1, 1};
    float m_angle = 0;
    
    bool m_is_dead = false;
    

private:

    std::function<void(int, ObjectType)> m_on_destruction_callback = [](int, ObjectType){};
    ObjectType m_type;
};
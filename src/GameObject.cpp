#include "GameObject.h"

#include "Polygon.h"

#include <Texture.h>
#include "magic_enum.hpp"
#include "magic_enum_utility.hpp"

GameObject::GameObject(GameWorld* world, TextureHolder& textures, ObjectType type)
 :
  m_world(world), m_textures(&textures), m_type(type)
  {
    m_name = magic_enum::enum_name(type); 
  }

void GameObject::updateAll(float dt)
{
    if (m_collision_shape)
    {
        m_collision_shape->setPosition(m_pos.x, m_pos.y);
        m_collision_shape->setRotation(m_angle);
    }
    m_pos += m_vel * dt;
}

    void GameObject::move(utils::Vector2f by)
    {
        m_pos += by;
        if(m_collision_shape)
        {
            m_collision_shape->setPosition(m_pos);
        }
    }

bool GameObject::collides() const
{
   return m_collision_shape != nullptr;
}

const utils::Vector2f& GameObject::getPosition() const
{
    return m_pos;
}

float GameObject::getX() const
{
    return m_pos.x;
}

float GameObject::getY() const
{
    return m_pos.y;
}

void GameObject::setY(float y)
{
    m_pos.y = y;
}

void GameObject::setX(float x)
{
    m_pos.x = x;
}
GameObject* GameObject::getTarget() const
{
    return m_target;
}

void GameObject::setTarget(GameObject* target)
{
    m_target = target;
}



float GameObject::getAngle() const
{
    return m_angle;
}

Polygon &GameObject::getCollisionShape()
{
    return *m_collision_shape;
}


bool GameObject::doesPhysics()const
{
    return m_rigid_body != nullptr;
}

RigidBody &GameObject::getRigidBody()
{
    return *m_rigid_body;
}


int GameObject::getId() const
{
    return m_id;
}
ObjectType GameObject::getType() const
{
    return m_type;
}

    void GameObject::removeCollider()
    {
        if(m_collision_shape)
        {
            m_collision_shape = nullptr;
        }
    }

    void GameObject::kill()
    {
        m_is_dead = true;
    }

    bool GameObject::isDead()const
    {
        return m_is_dead;
    }


    void GameObject::setPosition(utils::Vector2f new_position)
    {
        m_pos = new_position;
        if(m_collision_shape)
        {
            m_collision_shape->setPosition(new_position);
        }
    }

    void GameObject::setSize(utils::Vector2f size)
    {
        if(m_collision_shape)
        {
            m_collision_shape->setScale(size);
        }
        m_size = size;
    }

    void GameObject::setAngle(float angle)
    {
        m_angle = angle;
        if(m_collision_shape)
        {
            m_collision_shape->setRotation(angle);
        }
    }


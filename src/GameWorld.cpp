#include "GameWorld.h"

#include <chrono>

#include <Renderer.h>
#include <Utils/IO.h>

#include "Entities.h"
// #include "Entities/Enemy.h"
// #include "Entities/Player.h"
// #include "Entities/Attacks.h"

#include "magic_enum.hpp"
#include "magic_enum_utility.hpp"

GameWorld::GameWorld()
{

    loadTextures();

    // m_neighbour_searcher = std::make_unique<GridNeighbourSearcher>();
}

std::shared_ptr<GameObject> GameWorld::addObject(std::string obj_type_text)
{
    auto obj_type = magic_enum::enum_cast<ObjectType>(obj_type_text);
    if (obj_type.has_value())
    {
        return addObject(obj_type .value());
    }
    return nullptr;
}
std::shared_ptr<GameObject> GameWorld::addObject(ObjectType type)
{
    std::shared_ptr<GameObject> new_object;

    switch (type)
    {
    case ObjectType::Player:
        new_object = std::make_shared<PlayerEntity>(this, m_textures, ObjectType::Player);
        break;
    case ObjectType::Bullet:
        new_object = std::make_shared<Projectile>(this, m_textures, ObjectType::Bullet);
        break;
    default:
        throw std::runtime_error("You forgot to add the new object here!");
    }

    m_to_add.push(new_object);
    return m_to_add.back();
}

void GameWorld::addQueuedEntities()
{
    while (!m_to_add.empty())
    {
        auto new_object = m_to_add.front();
        auto new_id = m_entities.addObject(new_object);
        m_entities.at(new_id)->m_id = new_id;
        if (m_entities.at(new_id)->collides())
        {
            m_collision_system.insertObject(m_entities.at(new_id));
        }

        m_entities.at(new_id)->onCreation();
        m_to_add.pop();
    }
}

void GameWorld::removeQueuedEntities()
{
    while (!m_to_destroy.empty())
    {
        auto object = m_to_destroy.front();
        object->onDestruction();

        if (object->collides())
        {
            m_collision_system.removeObject(*object);
        }
        m_entities.remove(object->getId());
        m_to_destroy.pop();
    }
}

void GameWorld::destroyObject(int entity_id)
{
    //! notify observers of destruction
    std::weak_ptr<GameObject> e = m_entities.at(entity_id);
    m_entity_destroyed_subject.notify(e);

    m_to_destroy.push(m_entities.at(entity_id));
}

void GameWorld::update(float dt)
{

    m_collision_system.update();

    for (auto &obj : m_entities.getObjects())
    {
        obj->updateAll(dt);
        if (obj->isDead())
        {
            destroyObject(obj->getId());
        }
    }

    addQueuedEntities();
    removeQueuedEntities();
}

void GameWorld::draw(LayersHolder &layers)
{
    for (auto &obj : m_entities.getObjects())
    {
        obj->draw(layers);
    }
}

int GameWorld::addEntityCallback(EntityEventType type, std::function<void(std::weak_ptr<GameObject>)> callback)
{
    if (type == EntityEventType::EntityDestroyed)
    {
        return m_entity_destroyed_subject.attach(callback);
    }
    else
    {
        return m_entity_created_subject.attach(callback);
    }
}

void GameWorld::removeEntityCallback(EntityEventType type, int callback_id)
{
    if (type == EntityEventType::EntityDestroyed)
    {
        m_entity_destroyed_subject.detach(callback_id);
    }
    else
    {
        m_entity_created_subject.detach(callback_id);
    }
}
void GameWorld::loadTextures()
{
    std::filesystem::path resources_path{"../Resources/"};

    auto texture_filenames = extractNamesInDirectory(resources_path, ".png");
    for (auto &texture_filename : texture_filenames)
    {
        auto pos_right = texture_filename.find_last_of('.');
        std::string texture_name = texture_filename.substr(0, pos_right);
        m_textures.add(texture_name, "../Resources/" + texture_filename);
    }
}
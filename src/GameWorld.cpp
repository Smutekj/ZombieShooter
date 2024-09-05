#include "GameWorld.h"

#include <chrono>

#include <Renderer.h>
#include <Utils/IO.h>

#include "Entities.h"
#include "Enviroment.h"
// #include "Entities/Enemy.h"
// #include "Entities/Player.h"
// #include "Entities/Attacks.h"

#include "magic_enum.hpp"
#include "magic_enum_utility.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/basic_file_sink.h>

using namespace boost;
using EntityList = mp11::mp_list<ConcreteEntityHolder<PlayerEntity>, ConcreteEntityHolder<Enemy>>;
using EntityList2 = mp11::mp_push_back<EntityList, ConcreteEntityHolder<Projectile>>;

using DEntityList = mp11::mp_list<DynamicEntityHolder<PlayerEntity>, DynamicEntityHolder<Enemy>>;
using DEntityList2 = mp11::mp_push_back<DEntityList, DynamicEntityHolder<Projectile>>;

GameWorld::GameWorld()
{

    loadTextures();

    // m_neighbour_searcher = std::make_unique<GridNeighbourSearcher>();
}

std::shared_ptr<GameObject> GameWorld::addObject(std::string obj_type_text, std::string name, int parent_id)
{
    auto obj_type = magic_enum::enum_cast<ObjectType>(obj_type_text);
    if (obj_type.has_value())
    {
        return addObject(obj_type.value(), name, parent_id);
    }
    std::string msg = " OBJECT NOT CREATED! ObjType: " + obj_type_text + " Does not exist!";
    spdlog::get("lua_logger")->error(msg);
    return nullptr;
}

// template <class T>
// void addToList(std::shared_ptr<T> obj)
// {
//     using I = mp11::mp_find<EntityList2, ConcreteEntityHolder<T>>;
//     using EntityCont = mp11::mp_at_c<EntityList2, I>;
//     EntityCont::m_entities.at(0) = *obj;
// }

std::shared_ptr<GameObject> GameWorld::addObject(ObjectType type, std::string object_name, int parent_id)
{
    std::shared_ptr<GameObject> new_object;

    switch (type)
    {
    case ObjectType::Player:
        new_object = std::make_shared<PlayerEntity>(this, m_textures);
        break;
    case ObjectType::Bullet:
        new_object = std::make_shared<Projectile>(this, m_textures);
        break;
    case ObjectType::Enemy:
        new_object = std::make_shared<Enemy>(this, m_textures, m_collision_system, m_player);
        break;
    case ObjectType::Orbiter:
        new_object = std::make_shared<OrbitingShield>(this, m_textures);
        break;
    default:
        throw std::runtime_error("You forgot to add the new object here!");
    }

    new_object->m_name = object_name;
    m_to_add.push({new_object, object_name, parent_id});
    return m_to_add.back().p_object;
}

void GameWorld::addQueuedEntities()
{
    while (!m_to_add.empty())
    {
        auto new_object = m_to_add.front().p_object;
        auto new_name = m_to_add.front().name;
        auto new_parent = m_to_add.front().parent;
        m_to_add.pop();

        //! THIS NEEDS TO BE FIRST!!!
        auto new_id = m_entities.addObject(new_object);
        m_entities.at(new_id)->m_id = new_id;
        m_scene.addObject(new_object);

        if (new_parent != -1)
        {
            m_scene.changeParentOf(new_id, new_parent);
        }

        if (m_entities.at(new_id)->collides())
        {
            m_collision_system.insertObject(m_entities.at(new_id));
        }

        m_entities.at(new_id)->onCreation();
        m_name2id[new_name] = new_id;
    }
}

void GameWorld::removeQueuedEntities()
{
    while (!m_to_destroy.empty())
    {
        auto object = m_to_destroy.front().p_object;
        auto name = m_to_destroy.front().name;
        object->onDestruction();

        if (object->collides())
        {
            m_collision_system.removeObject(*object);
        }
        m_scene.removeObjectAndChildren(object->getId());
        m_entities.remove(object->getId());
        m_name2id.erase(name);
        m_to_destroy.pop();
    }
}

void GameWorld::destroyObject(int entity_id)
{
    //! notify observers of destruction
    std::weak_ptr<GameObject> e = m_entities.at(entity_id);
    m_entity_destroyed_subject.notify(e);

    m_to_destroy.push({
        m_entities.at(entity_id),
    });
}

void GameWorld::update(float dt)
{

    m_collision_system.update();

    m_scene.update(dt);

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
//
// static ConcreteEntityHolder<PlayerEntity> s_player_holder;
// static ConcreteEntityHolder<Enemy> s_enemy_holder;

constexpr void GameWorld::update2()
{

    mp11::mp_for_each<EntityList2>([&](auto entity_holder)
                                   {
                                    auto& entities = decltype(entity_holder)::m_entities;
                                    // using decltype(entity_holder)
                                      for (auto &e : entities)
                                      {
                                        e.update(0.01f);
                                      } });
}

int GameWorld::getIdOf(const std::string &name) const
{
    if (m_name2id.count(name) == 0)
    {
        return -1;
    }
    return m_name2id.at(name);
}

std::shared_ptr<GameObject> GameWorld::get(int entity_id) const
{
    if (entity_id == -1)
    {
        spdlog::get("lua_logger")->error(" Parent object with index: " + std::to_string(entity_id) + " does not exist ");
        return nullptr;
    }
    return m_entities.at(entity_id);
}
template <class EntityType>
std::shared_ptr<EntityType> GameWorld::get(const std::string &name) const
{
    auto obj_ptr = get(name);
    if (obj_ptr)
    {
        assert(dynamic_cast<EntityType *>(obj_ptr) != nullptr);
        return std::shared_ptr<EntityType>{static_cast<EntityType &>(*obj_ptr)};
    }
    return nullptr;
}
std::shared_ptr<GameObject> GameWorld::get(const std::string &name) const
{
    if (m_name2id.count(name) == 0)
    {
        spdlog::get("lua_logger")->error(" Object with name: " + std::string(name) + " does not exist ");
        return nullptr;
    }
    return get(m_name2id.at(name));
}



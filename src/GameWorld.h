#pragma once

#include <Grid.h>
#include <Triangulation.h>

#include <unordered_map>
#include <functional>
#include <queue>
// #include <boost/mpl/for_each.hpp>
// #include <boost/mpl/set.hpp>
// #include <boost/mp11/algorithm.hpp>

#include "CollisionSystem.h"
#include "PathFinding/PathFinder.h"
#include "Utils/ObjectPool.h"
#include "Enviroment.h"
#include "SceneGraph.h"
// #include "GridNeighbourSearcher.h"

namespace map
{
    constexpr int MAP_SIZE_X = 1500;
    constexpr int MAP_SIZE_Y = 1500;
    constexpr int MAP_GRID_CELLS_X = 25;
    constexpr int MAP_GRID_CELLS_Y = MAP_GRID_CELLS_X;
} //! namespace map

class Renderer;

template <class... FuncArgs>
class EventSubject
{
    // template <class FuncArgs...>
    // using void(FuncArgs...) = CallBackType;

public:
    void notify(FuncArgs... args)
    {
        for (auto &callback : m_observers.getObjects())
        {
            callback(args...);
        }
    }

    int attach(std::function<void(FuncArgs...)> observer)
    {
        auto observer_id = m_observers.addObject(observer);
        return observer_id;
    }

    void detach(int observer_id)
    {
        m_observers.remove(observer_id);
    }

private:
    utils::DynamicObjectPool<std::function<void(FuncArgs...)>, 50> m_observers;
};

using EntityEvent = EventSubject<std::weak_ptr<GameObject>>;

class PlayerEntity;

enum class EntityEventType
{
    EntityDestroyed,
    EntityCreated,
};

class EntityHolder
{

private:
};

// constexpr utils::DynamicObjectPool<int, 5000> m_entities;

template <class EntityType>
struct ConcreteEntityHolder : public EntityHolder
{
    // boost::mpl::liste
    // static utils::DynamicObjectPool<EntityType, 5000> m_entities;
    static std::array<EntityType, 5000> m_entities;
};
template <class EntityType>
struct DynamicEntityHolder : public EntityHolder
{
    // boost::mpl::liste
    // static utils::DynamicObjectPool<EntityType, 5000> m_entities;
    std::unordered_map<std::string, std::shared_ptr<EntityType>> m_entities;
};

class EntityBookKeeper
{

    template <class EntityType>
    void addEntity(std::shared_ptr<EntityType> p_entity)
    {
    }

    template <class EntityType>
    std::shared_ptr<EntityType> get(ObjectType type_id, std::string name)
    {
        auto &holder = m_entity_map.at(type_id);
        if (!holder)
        {
            return nullptr;
        }
        auto &entities = static_cast<DynamicEntityHolder<EntityType> &>(holder).m_entities;
        if (entities.count(name) == 0)
        {
            return nullptr;
        }

        return entities.at(name);
    }

public:
    std::unordered_map<ObjectType, std::unique_ptr<EntityHolder>> m_entity_map;
};


struct NewObjectData
{
    std::shared_ptr<GameObject> p_object = nullptr;
    std::string name = "DefaultName";
    int parent = -1;
};

constexpr int N_MAX_ENTITIES = 10000;

namespace spdlog
{
    class logger;
};

class GameWorld
{

    constexpr void update2();

    GameWorld();
    GameWorld(GameWorld const &) = delete;
    void operator=(GameWorld const &) = delete;

public:
    static GameWorld &getWorld()
    {
        static GameWorld instance;
        return instance;
    }

    const std::unordered_map<std::string, int>& getNames() const
    {
        return m_name2id;
    }

    void destroyObject(int entity_id);
    std::shared_ptr<GameObject> addObject(ObjectType type, std::string name, int parent_id = -1);
    std::shared_ptr<GameObject> addObject(std::string type, std::string name, int parent_id = -1);

    template <class EntityType, class... Args>
    std::shared_ptr<EntityType> addObject(const std::string name, Args... args);

    template <class TriggerType, class... Args>
    TriggerType &addTrigger(Args... args);

    template <class EffectType, class... Args>
    EffectType &addVisualEffect(const std::string name, Args &&...args);

    std::shared_ptr<EnviromentEffect> addEffect(std::string obj_type_text, std::string name, int parent_id = -1);

    void update(float dt);
    void draw(LayersHolder &layers);

    int addEntityCallback(EntityEventType type, std::function<void(std::weak_ptr<GameObject>)> callback);
    void removeEntityCallback(EntityEventType type, int callback_id);

    int getIdOf(const std::string &name) const;
    const std::string getName(int entity_id) const;
    bool setName(int entity_id, std::string new_name);
    std::shared_ptr<GameObject> get(int entity_id) const;
    std::shared_ptr<GameObject> get(const std::string name) const;
    template <class EntityType>
    std::shared_ptr<EntityType> get(const std::string &name) const;

    bool kill(const std::string name);
    cdt::Triangulation<cdt::Vector2i> &getTriangulation()
    {
        assert(m_cdt);
        return *m_cdt;
    }
    TextureHolder& getTextrures() 
    {
        return m_textures;
    }
    pathfinding::PathFinder &getPathFinder()
    {
        assert(m_pathfinder);
        return *m_pathfinder;
    }

    Collisions::CollisionSystem& getCollider()
    {
        return m_collision_system;
    }

    Font* getFont() 
    {
        return m_font.get();
    }

private:
    void addQueuedEntities();
    void removeQueuedEntities();
    void loadTextures();

public:
    SceneGraph m_scene;
    ShaderHolder m_shaders;

private:
    EntityEvent m_entity_destroyed_subject;
    EntityEvent m_entity_created_subject;
    utils::DynamicObjectPool<std::shared_ptr<GameObject>, N_MAX_ENTITIES> m_entities;

    std::unordered_map<std::string, int> m_name2id;
    std::unordered_map<int, std::string> m_id2name;
    std::unordered_map<std::string, std::unordered_set<int>> m_name2ids;

    // std::unique_ptr<GridNeighbourSearcher> m_neighbour_searcher;
    Collisions::CollisionSystem m_collision_system;
    std::shared_ptr<cdt::Triangulation<cdt::Vector2i>> m_cdt = nullptr;
    std::shared_ptr<pathfinding::PathFinder> m_pathfinder = nullptr;

    std::queue<NewObjectData> m_to_add;
    std::queue<NewObjectData> m_to_destroy;

    TextureHolder m_textures;
    std::shared_ptr<Font> m_font = nullptr;

    PlayerEntity *m_player;

    std::shared_ptr<spdlog::logger> m_logger;
};

template <class TriggerType, class... Args>
TriggerType &GameWorld::addTrigger(Args... args)
{
    auto new_trigger = std::make_shared<TriggerType>(this, m_textures, args...);
    m_to_add.push(new_trigger);
    return *new_trigger;
}

template <class EffectType, class... Args>
EffectType &GameWorld::addVisualEffect(const std::string name, Args &&...args)
{
    // static_assert(std::is_base_of_v<EnviromentEffect, EffectType>());
    NewObjectData new_obj;
    auto ptr_obj = std::make_shared<EffectType>(m_shaders, m_textures, args...);
    new_obj.p_object = ptr_obj;
    new_obj.name = name;
    new_obj.parent = -1;
    m_to_add.push(new_obj);
    return *ptr_obj;
}
template <class EntityType, class... Args>
std::shared_ptr<EntityType> GameWorld::addObject(const std::string name, Args... args)
{
    // static_assert(std::is_base_of_v<EnviromentEffect, EffectType>());
    NewObjectData new_obj;
    // if(m_name2id.count(name) > 0)
    // {
    //     return nullptr;
    // }
    auto ptr_obj = std::make_shared<EntityType>(m_textures, args...);
    new_obj.p_object = ptr_obj;
    new_obj.name = name;
    new_obj.parent = -1;
    m_to_add.push(new_obj);
    addQueuedEntities();
    return ptr_obj;
}

template <class EntityType>
std::shared_ptr<EntityType> GameWorld::get(const std::string &name) const
{
    auto obj_ptr = get(name);
    if (obj_ptr)
    {
        std::shared_ptr<EntityType> new_ptr = std::dynamic_pointer_cast<EntityType> (obj_ptr);
        return new_ptr;
    }
    return nullptr;
}

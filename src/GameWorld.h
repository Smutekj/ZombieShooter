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
// #include "GridNeighbourSearcher.h"

namespace map
{
    constexpr int MAP_SIZE_X = 5000;
    constexpr int MAP_SIZE_Y = 5000;
    constexpr int MAP_GRID_CELLS_X = 50;
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

struct SceneNode
{
    std::vector<int> children;
    GameObject *p_object = nullptr;
    int parent = -1;
};

struct ObjectIdentifier
{
    ObjectType type;
    std::string name;
};

struct SceneGraph
{
public:
    SceneGraph()
    {
    }

    void update(float dt)
    {
        for (auto &root : m_roots)
        {
            updateRoot(root, dt);
        }
    }

    void updateRoot(int root_ind, float dt)
    {
        auto &root = m_nodes.at(root_ind);
        root.p_object->update(dt);
        std::queue<int> to_visit;
        for (auto &child : root.children)
        {
            to_visit.push(child);
        }

        while (!to_visit.empty())
        {
            auto curr_ind = to_visit.front();
            auto &curr_node = m_nodes.at(curr_ind);
            to_visit.pop();

            for (auto &child : curr_node.children)
            {
                to_visit.push(child);
            }

            auto parent_obj = m_nodes.at(curr_node.parent).p_object;
            auto parent_pos = parent_obj->getPosition();
            curr_node.p_object->m_transform.transform(parent_pos);
            curr_node.p_object->setPosition(parent_pos);
            curr_node.p_object->update(dt);
        }
    }

    template <class GameObjType>
    bool addObject(std::shared_ptr<GameObjType> p_obj)
    {
        SceneNode new_node{{}, p_obj.get(), -1};
        auto new_scene_ind = m_nodes.addObject(new_node);
        m_roots.insert(new_scene_ind);
        m_obj_map[p_obj->getId()] = new_scene_ind;
        return true;
    }

    template <class GameObjType>
    bool addAsChildOf(int parent_entity_id, std::shared_ptr<GameObjType> new_obj)
    {
        auto parent_ind = m_obj_map.at(parent_entity_id);
        auto &parent_node = m_nodes.at(parent_ind);
        auto &children = parent_node.children;

        SceneNode new_node{{}, new_obj.get(), parent_ind};
        auto new_ind = m_nodes.addObject(new_node);

        assert(std::find(children.begin(), children.end(), new_ind) == children.end()); //! the index does not exist!
        children.push_back(new_ind);
        m_obj_map[new_obj->getId()] = new_ind;
        return true;
    }

    void changeParentOf(int entity_id, int new_parent_entity_id)
    {

        auto node_ind = getNodeId(entity_id);
        auto &node = getNode(entity_id);
        auto new_parent_node_ind = getNodeId(new_parent_entity_id);
        auto &new_parent_node = getNode(new_parent_entity_id);
        if (node.parent != -1)
        {
            auto &old_children = m_nodes.at(node.parent).children;
            assert(std::find(old_children.begin(), old_children.end(), node_ind) != old_children.end());
            old_children.erase(std::find(old_children.begin(), old_children.end(), node_ind));
        }

        new_parent_node.children.push_back(node_ind);
        node.parent = new_parent_node_ind;
        if (m_roots.contains(entity_id))
        {
            m_roots.erase(entity_id);
        }
    }

    SceneNode &getNode(int entity_id)
    {
        return m_nodes.at(getNodeId(entity_id));
    }
    int getNodeId(int entity_id)
    {
        int obj_ind = m_obj_map.at(entity_id);
        // assert(obj_ind < m_nodes.getObjects().size());
        return obj_ind;
    }

    // bool isLeaf(int entity_id)
    // {
    //     return getNode(entity_id).children.empty();
    // }
    bool isLeaf(int scene_node_ind)
    {
        return m_nodes.at(scene_node_ind).children.empty();
    }

    bool removeObject(int entity_id)
    {
        auto curr_obj_ind = getNodeId(entity_id);
        auto curr_parent_ind = m_nodes.at(curr_obj_ind).parent;
        if (curr_parent_ind != -1)
        {
            auto &parents_children = m_nodes.at(curr_parent_ind).children;
            auto removed_child_it = std::find(parents_children.begin(), parents_children.end(), curr_obj_ind);
            assert(removed_child_it != parents_children.end());
            parents_children.erase(removed_child_it);
        }

        //! old children will have parent of the removed obj
        auto &children = m_nodes.at(curr_obj_ind).children;
        for (auto &child : children)
        {
            m_nodes.at(child).parent = curr_parent_ind;
        }
        if (m_nodes.at(curr_obj_ind).parent == -1) //! if is root remove from roots
        {
            m_roots.erase(curr_obj_ind);
            for (auto &child : children) //! all children must therefore become roots themselves
            {
                m_roots.insert(child);
            }
        }
        m_nodes.remove(curr_obj_ind);
        return true;
    }

    bool removeObjectAndChildren(int entity_id)
    {
        auto curr_obj_ind = getNodeId(entity_id);
        auto curr_parent_ind = m_nodes.at(curr_obj_ind).parent;
        if (curr_parent_ind != -1) //! if he is not root we inform his parent of the removal
        {
            auto &parents_children = m_nodes.at(curr_parent_ind).children;
            auto removed_child_it = std::find(parents_children.begin(), parents_children.end(), curr_obj_ind);
            assert(removed_child_it != parents_children.end());
            parents_children.erase(removed_child_it);
        }
        else //! if is root remove from roots
        {
            m_roots.erase(curr_obj_ind); //! no need to add children as roots because they will be removed
        }

        std::queue<int> to_remove;
        to_remove.push({curr_obj_ind});
        while (!to_remove.empty())
        {
            curr_obj_ind = to_remove.front();
            to_remove.pop();

            auto &curr_obj = m_nodes.at(curr_obj_ind);

            //! do the same thing for all children
            auto &children = m_nodes.at(curr_obj_ind).children;
            for (auto &child : children)
            {
                to_remove.push(child);
            }
            //! remove from all references
            assert(m_roots.count(curr_obj_ind) == 0);
            m_obj_map.erase(entity_id);
            m_nodes.remove(curr_obj_ind);
        }
        return true;
    }

public:
    utils::DynamicObjectPool<SceneNode, 5000> m_nodes;
    std::unordered_map<int, int> m_obj_map;
    std::unordered_set<int> m_roots;
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

    const Collisions::CollisionSystem& getCollider() const
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
    if(m_name2id.count(name) > 0)
    {
        return nullptr;
    }
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

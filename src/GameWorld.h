#pragma once

#include "Utils/ObjectPool.h"
#include <Grid.h>

#include <unordered_map>
#include <functional>
#include <queue>

#include "CollisionSystem.h"
// #include "GridNeighbourSearcher.h"

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

class GameWorld
{

    GameWorld();
    GameWorld(GameWorld const&) = delete;
    void operator=(GameWorld const&) = delete;
public:
    static GameWorld &getWorld()
    {
        static GameWorld instance;
        return instance;
    }

    void destroyObject(int entity_id);
    std::shared_ptr<GameObject> addObject(ObjectType type);
    std::shared_ptr<GameObject> addObject(std::string type);

    template <class TriggerType, class... Args>
    TriggerType &addTrigger(Args... args);

    template <class EffectType, class... Args>
    EffectType &addVisualEffectT(Args... args);

    void update(float dt);
    void draw(LayersHolder &layers);

    int addEntityCallback(EntityEventType type, std::function<void(std::weak_ptr<GameObject>)> callback);
    void removeEntityCallback(EntityEventType type, int callback_id);

private:
    void addQueuedEntities();
    void removeQueuedEntities();
    void loadTextures();

private:
    EntityEvent m_entity_destroyed_subject;
    EntityEvent m_entity_created_subject;
    utils::DynamicObjectPool<std::shared_ptr<GameObject>, 5000> m_entities;

    // std::unique_ptr<GridNeighbourSearcher> m_neighbour_searcher;
    Collisions::CollisionSystem m_collision_system;

    std::queue<std::shared_ptr<GameObject>> m_to_add;
    std::queue<std::shared_ptr<GameObject>> m_to_destroy;

    TextureHolder m_textures;
    PlayerEntity *m_player;
};

template <class TriggerType, class... Args>
TriggerType &GameWorld::addTrigger(Args... args)
{
    auto new_trigger = std::make_shared<TriggerType>(this, m_textures, args...);
    m_to_add.push(new_trigger);
    return *new_trigger;
}

template <class EffectType, class... Args>
EffectType &GameWorld::addVisualEffectT(Args... args)
{
    auto new_effect = std::make_shared<EffectType>(this, m_textures, args...);
    m_to_add.push(new_effect);
    return *new_effect;
}
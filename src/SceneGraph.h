#pragma once


#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "Utils/ObjectPool.h"

#include "GameObject.h"

class GameObject;
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
    SceneGraph();

    void update(float dt);

    void updateRoot(int root_ind, float dt);

    template <class GameObjType>
    bool addObject(std::shared_ptr<GameObjType> p_obj);

    template <class GameObjType>
    bool addAsChildOf(int parent_entity_id, std::shared_ptr<GameObjType> new_obj);

    void changeParentOf(int entity_id, int new_parent_entity_id);

    SceneNode &getNode(int entity_id);
    int getNodeId(int entity_id);

    // bool isLeaf(int entity_id)
    // {
    //     return getNode(entity_id).children.empty();
    // }
    bool isLeaf(int scene_node_ind);

    bool removeObject(int entity_id);

    bool removeObjectAndChildren(int entity_id);

public:
    utils::DynamicObjectPool<SceneNode, 5000> m_nodes;
    std::unordered_map<int, int> m_obj_map;
    std::unordered_set<int> m_roots;
};

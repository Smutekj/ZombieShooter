#include "SceneGraph.h"

#include <queue>


SceneGraph::SceneGraph()
{
}

void SceneGraph::update(float dt)
{
    for (auto &root : m_roots)
    {
        updateRoot(root, dt);
    }
}

void SceneGraph::updateRoot(int root_ind, float dt)
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
bool SceneGraph::addObject(std::shared_ptr<GameObjType> p_obj)
{
    SceneNode new_node{{}, p_obj.get(), -1};
    auto new_scene_ind = m_nodes.addObject(new_node);
    m_roots.insert(new_scene_ind);
    m_obj_map[p_obj->getId()] = new_scene_ind;
    return true;
}

template <class GameObjType>
bool SceneGraph::addAsChildOf(int parent_entity_id, std::shared_ptr<GameObjType> new_obj)
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

void SceneGraph::changeParentOf(int entity_id, int new_parent_entity_id)
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
    if (m_roots.count(entity_id) > 0)
    {
        m_roots.erase(entity_id);
    }
}

SceneNode &SceneGraph::getNode(int entity_id)
{
    return m_nodes.at(getNodeId(entity_id));
}
int SceneGraph::getNodeId(int entity_id)
{
    int obj_ind = m_obj_map.at(entity_id);
    // assert(obj_ind < m_nodes.getObjects().size());
    return obj_ind;
}

// bool isLeaf(int entity_id)
// {
//     return getNode(entity_id).children.empty();
// }
bool SceneGraph::isLeaf(int scene_node_ind)
{
    return m_nodes.at(scene_node_ind).children.empty();
}

bool SceneGraph::removeObject(int entity_id)
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

bool SceneGraph::removeObjectAndChildren(int entity_id)
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
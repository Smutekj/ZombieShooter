#pragma once

#include "BVH.h"

#include <memory>
#include <vector>
#include <unordered_set>
#include <variant>

#include <Utils/Vector2.h>
#include "GameObject.h"
#include "Polygon.h"

namespace Collisions
{

    template <int N_VERTS>
    struct ConvexPolygon : public Transform
    {
        std::array<utils::Vector2f, N_VERTS> vertices;
    };

    struct Circle : public Transform
    {


    };

    struct Edge
    {
        utils::Vector2f from;
        utils::Vector2f t;
        float l;
        Edge() = default;
        Edge(utils::Vector2f from, utils::Vector2f to) : 
        from(from)
        {
            t = to - from;
            l = norm(t);   
            t /= l;
        }
        utils::Vector2f to() const { return from + t * l; }
    };

    struct CollisionFeature
    {
        utils::Vector2f best_vertex;
        Edge edge;
    };

    CollisionData inline calcCollisionData(const std::vector<utils::Vector2f> &points1,
                                           const std::vector<utils::Vector2f> &points2);

    class CollisionSystem
    {

        std::unordered_map<ObjectType, BoundingVolumeTree> m_object_type2tree;
        std::unordered_map<int, std::weak_ptr<GameObject>> m_objects;

        struct pair_hash
        {
            inline std::size_t operator()(const std::pair<int, int> &v) const
            {
                return v.first * 31 + v.second;
            }
        };

        std::unordered_set<std::pair<int, int>, pair_hash> m_collided;
        std::unordered_set<std::pair<int, int>, pair_hash> m_exceptions;

    public:
        CollisionSystem();

        void insertObject(std::shared_ptr<GameObject> &p_object);
        void removeObject(GameObject &object);
        void update();
        std::vector<int> findNearestObjectInds(ObjectType type, utils::Vector2f center, float radius) const;
        std::vector<GameObject *> findNearestObjects(ObjectType type, utils::Vector2f center, float radius) const;
        std::vector<GameObject *> findNearestObjects(ObjectType type, AABB colllision_rect) const;
        std::vector<GameObject *> findNearestObjects(AABB colllision_rect) const;
        utils::Vector2f findClosestIntesection(ObjectType type, utils::Vector2f at, utils::Vector2f dir, float length) ;
        GameObject* findClosestObject(ObjectType type, utils::Vector2f at, utils::Vector2f dir, float length) ;

    private:
        void narrowPhase(const std::vector<std::pair<int, int>> &colliding_pairs);
        CollisionData getCollisionData(Polygon &pa, Polygon &pb) const;
    };

    CollisionData inline calcCollisionData(const std::vector<utils::Vector2f> &points1,
                                           const std::vector<utils::Vector2f> &points2);
    int inline furthestVertex(utils::Vector2f separation_axis, const std::vector<utils::Vector2f> &points);
    CollisionFeature inline obtainFeatures(const utils::Vector2f axis, const std::vector<utils::Vector2f> &points);
    std::vector<utils::Vector2f> inline clip(utils::Vector2f v1, utils::Vector2f v2, utils::Vector2f n, float overlap);

    std::vector<utils::Vector2f> inline clipEdges(
        CollisionFeature &ref_features,
        CollisionFeature &inc_features,
        utils::Vector2f n);

    void inline bounce(GameObject &obj1, GameObject &obj2, CollisionData c_data);

} //! namespace Collisions

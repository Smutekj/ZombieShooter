#include "CollisionSystem.h"

#include "GameObject.h"

namespace Collisions
{

    CollisionSystem::CollisionSystem()
    {
        for (int i = 0; i < static_cast<int>(ObjectType::Count); ++i)
        {
            m_object_type2tree[static_cast<ObjectType>(i)] = {};
        }
        m_exceptions.insert({(int)ObjectType::Wall, (int)ObjectType::Wall});
    }

    void CollisionSystem::insertObject(std::shared_ptr<GameObject> &p_obj)
    {
        assert(p_obj);
        auto &object = *p_obj;
        auto bounding_rect = object.getCollisionShape().getBoundingRect().inflate(1.2f);
        m_object_type2tree[object.getType()].addRect(bounding_rect, object.getId());

        assert(m_objects.count(object.getId()) == 0);
        m_objects[object.getId()] = p_obj;
    }

    void CollisionSystem::removeObject(GameObject &object)
    {
        m_object_type2tree.at(object.getType()).removeObject(object.getId());

        assert(m_objects.count(object.getId()) > 0);

        m_objects.erase(object.getId());
    }

    void CollisionSystem::update()
    {

        for (auto &[ind, p_entity] : m_objects)
        {
            auto &entity = *p_entity.lock();
            auto type = entity.getType();
            auto &tree = m_object_type2tree.at(type);
            auto fitting_rect = entity.getCollisionShape().getBoundingRect();
            auto big_bounding_rect = tree.getObjectRect(ind);

            //! if object moved in a way that rect in the collision tree does not fully contain it
            if (makeUnion(fitting_rect, big_bounding_rect).volume() > big_bounding_rect.volume())
            {
                tree.removeObject(ind);
                tree.addRect(fitting_rect.inflate(1.2f), ind);
            }
        }

        for (int i = 0; i < static_cast<int>(ObjectType::Count); ++i)
        {

            auto &tree_i = m_object_type2tree.at(static_cast<ObjectType>(i));
            for (int j = i; j < static_cast<int>(ObjectType::Count); ++j)
            { //! for all pairs of object trees;
                if (m_exceptions.count({i, j}) > 0)
                {
                    continue;
                }
                auto &tree_j = m_object_type2tree.at(static_cast<ObjectType>(j));

                auto close_pairs = tree_i.findClosePairsWith(tree_j);

                narrowPhase(close_pairs);
            }
        }
        m_collided.clear();
    }

    void CollisionSystem::narrowPhase(const std::vector<std::pair<int, int>> &colliding_pairs)
    {
        for (auto [i1, i2] : colliding_pairs)
        {

            auto &obj1 = *m_objects.at(i1).lock();
            auto &obj2 = *m_objects.at(i2).lock();

            if (m_collided.count({i1, i2}) > 0)
            {
                continue;
            }
            m_collided.insert({i1, i2});

            CollisionData collision_data;
            collision_data = getCollisionData(obj1.getCollisionShape(), obj2.getCollisionShape());

            if (collision_data.minimum_translation > 0) //! there is a collision
            {
                // if (obj1.doesPhysics() && obj2.doesPhysics()) //! if both objects have rigid bodies we do physics
                // {
                //     bounce(obj1, obj2, collision_data);
                // }

                obj1.onCollisionWith(obj2, collision_data);
                obj2.onCollisionWith(obj1, collision_data);
            }
        }
    }

    CollisionData CollisionSystem::getCollisionData(Polygon &pa, Polygon &pb) const
    {
        auto points_a = pa.getPointsInWorld();
        auto points_b = pb.getPointsInWorld();
        auto c_data = calcCollisionData(points_a, points_b);

        if (c_data.minimum_translation < 0.f)
        {
            return c_data; //! there is no collision so we don't need to extract manifold
        }
        auto center_a = pa.getPosition();
        auto center_b = pb.getPosition();
        //! make separation axis point always from a to b
        auto are_flipped = dot((center_a - center_b), c_data.separation_axis) > 0;
        if (are_flipped)
        {
            c_data.separation_axis *= -1.f;
        }

        // auto col_feats1 = obtainFeatures(c_data.separation_axis, points_a);
        // auto col_feats2 = obtainFeatures(-1.f * c_data.separation_axis, points_b);

        // auto clipped_edge = clipEdges(col_feats1, col_feats2, c_data.separation_axis);
        // if (clipped_edge.size() == 0) //! clipping failed so we don't do collision
        // {
        //     c_data.minimum_translation = -1.f;
        //     return c_data;
        // }
        // for (auto ce : clipped_edge)
        // {
        //     c_data.contact_point += ce;
        // }
        // c_data.contact_point /= (float)clipped_edge.size();

        return c_data;
    }

    std::vector<int> CollisionSystem::findNearestObjectInds(ObjectType type, utils::Vector2f center, float radius) const
    {
        auto &tree = m_object_type2tree.at(type);

        AABB collision_rect({center - utils::Vector2f{radius, radius}, center + utils::Vector2f{radius, radius}});
        return tree.findIntersectingLeaves(collision_rect);
    }

    std::vector<GameObject *> CollisionSystem::findNearestObjects(ObjectType type, utils::Vector2f center, float radius) const
    {
        auto &tree = m_object_type2tree.at(type);

        AABB collision_rect({center - utils::Vector2f{radius, radius}, center + utils::Vector2f{radius, radius}});
        auto nearest_inds = tree.findIntersectingLeaves(collision_rect);
        std::vector<GameObject *> objects;
        for (auto ind : nearest_inds)
        {
            auto &obj = *m_objects.at(ind).lock();
            auto mvt = obj.getCollisionShape().getMVTOfSphere(center, radius);
            if (norm2(mvt) > 0.001f)
            {
                objects.push_back(&obj);
            }
        }
        return objects;
    }
    std::vector<GameObject *> CollisionSystem::findNearestObjects(ObjectType type, AABB collision_rect) const
    {
        auto &tree = m_object_type2tree.at(type);
        Polygon p2(4);
        p2.setPosition(collision_rect.getCenter());
        p2.setScale(collision_rect.getSize() / 2.f);
        auto nearest_inds = tree.findIntersectingLeaves(collision_rect);
        std::vector<GameObject *> objects;
        for (auto ind : nearest_inds)
        {
            auto &obj = *m_objects.at(ind).lock();
            auto collision_data = getCollisionData(obj.getCollisionShape(), p2);
            if (collision_data.minimum_translation > 0) //! there is a collision
            {
                objects.push_back(&obj);
            }
        }
        return objects;
    }
    std::vector<GameObject *> CollisionSystem::findNearestObjects(AABB collision_rect) const
    {

        std::vector<GameObject *> objects;
        auto n_types = static_cast<int>(ObjectType::Count);
        for (int i = 0; i < n_types; ++i)
        {
            auto &tree = m_object_type2tree.at(static_cast<ObjectType>(i));
            Polygon p2(4);
            p2.setPosition(collision_rect.getCenter());
            p2.setScale(collision_rect.getSize() / 2.f);
            auto nearest_inds = tree.findIntersectingLeaves(collision_rect);
            for (auto ind : nearest_inds)
            {
                auto &obj = *m_objects.at(ind).lock();
                // auto collision_data = getCollisionData(obj.getCollisionShape(), p2);
                // if (collision_data.minimum_translation > 0) //! there is a collision
                {
                    objects.push_back(&obj);
                }
            }
        }
            return objects;
    }

    utils::Vector2f CollisionSystem::findClosestIntesection(ObjectType type, utils::Vector2f at, utils::Vector2f dir, float length)
    {
        utils::Vector2f closest_intersection = at + dir * length;
        float min_dist = 200.f;
        auto inters = m_object_type2tree.at(type).rayCast(at, dir, length);
        for (auto ent_ind : inters)
        {
            auto &obj = *m_objects.at(ent_ind).lock();
            auto points = obj.getCollisionShape().getPointsInWorld();

            int next = 1;
            for (int i = 0; i < points.size(); ++i)
            {
                utils::Vector2f r1 = points.at(i);
                utils::Vector2f r2 = points.at(next);

                utils::Vector2f segment_intersection;
                if (utils::segmentsIntersect(r1, r2, at, at + dir * length, segment_intersection))
                {
                    auto new_dist = dist(segment_intersection, at);
                    if (new_dist < min_dist)
                    {
                        closest_intersection = segment_intersection;
                        min_dist = new_dist;
                    }
                }
                next++;
                if (next == points.size())
                {
                    next = 0;
                }
            }
        }
        return closest_intersection;
    }

    CollisionData inline calcCollisionData(const std::vector<utils::Vector2f> &points1,
                                           const std::vector<utils::Vector2f> &points2)
    {
        CollisionData collision_result;

        int next = 1;
        const auto n_points1 = points1.size();
        const auto n_points2 = points2.size();

        Edge contact_edge;

        float min_overlap = std::numeric_limits<float>::max();
        utils::Vector2f &min_axis = collision_result.separation_axis;
        for (int curr = 0; curr < n_points1; ++curr)
        {

            auto t1 = points1[next] - points1[curr]; //! line perpendicular to current polygon edge
            utils::Vector2f n1 = {t1.y, -t1.x};
            if (utils::approx_equal_zero(norm2(n1)))
            {
                continue;
            }
            n1 /= norm(n1);
            auto proj1 = projectOnAxis(n1, points1);
            auto proj2 = projectOnAxis(n1, points2);

            if (!overlap1D(proj1, proj2))
            {
                collision_result.minimum_translation = -1;
                return collision_result;
            }
            else
            {
                auto overlap = calcOverlap(proj1, proj2);
                if (utils::approx_equal_zero(overlap))
                {
                    continue;
                }
                if (overlap < min_overlap)
                {
                    min_overlap = overlap;
                    min_axis = n1;
                }
            }

            next++;
            if (next == n_points1)
            {
                next = 0;
            }
        }
        next = 1;
        for (int curr = 0; curr < n_points2; ++curr)
        {

            auto t1 = points2[next] - points2[curr]; //! line perpendicular to current polygon edge
            utils::Vector2f n1 = {t1.y, -t1.x};
            if (utils::approx_equal_zero(norm2(n1)))
            {
                continue;
            }
            n1 /= norm(n1);
            auto proj2 = projectOnAxis(n1, points2);
            auto proj1 = projectOnAxis(n1, points1);

            if (!overlap1D(proj1, proj2))
            {
                collision_result.minimum_translation = -1;
                return collision_result;
            }
            else
            {
                auto overlap = calcOverlap(proj1, proj2);
                if (utils::approx_equal_zero(overlap))
                {
                    continue;
                }
                if (overlap < min_overlap)
                {
                    min_overlap = overlap;
                    min_axis = n1;
                    collision_result.belongs_to_a = false;
                }
            }

            next++;
            if (next == n_points2)
            {
                next = 0;
            }
        }

        collision_result.minimum_translation = min_overlap;
        return collision_result;
    }

    int inline furthestVertex(utils::Vector2f separation_axis, const std::vector<utils::Vector2f> &points)
    {
        float max_dist = -std::numeric_limits<float>::max();
        int index = -1;
        for (int i = 0; i < points.size(); ++i)
        {
            auto dist = dot(points[i], separation_axis);
            if (dist > max_dist)
            {
                index = i;
                max_dist = dist;
            }
        }

        return index;
    }

    CollisionFeature inline obtainFeatures(const utils::Vector2f axis, const std::vector<utils::Vector2f> &points)
    {

        const auto n_points = points.size();
        auto furthest_v_ind1 = furthestVertex(axis, points);

        auto v1 = points[furthest_v_ind1];
        auto v1_next = points[(furthest_v_ind1 + 1) % n_points];
        auto v1_prev = points[(furthest_v_ind1 - 1 + n_points) % n_points];

        auto from_next = v1 - v1_next;
        auto from_prev = v1 - v1_prev;
        from_next /= norm(from_next);
        from_prev /= norm(from_prev);
        Edge best_edge;
        if (dot(from_prev, axis) <= dot(from_next, axis))
        {
            best_edge = Edge(v1_prev, v1);
        }
        else
        {
            best_edge = Edge(v1, v1_next);
        }
        CollisionFeature feature = {v1, best_edge};
        return feature;
    }

    std::vector<utils::Vector2f> inline clip(utils::Vector2f v1, utils::Vector2f v2, utils::Vector2f n, float overlap)
    {

        std::vector<utils::Vector2f> cp;
        float d1 = dot(v1, n) - overlap;
        float d2 = dot(v2, n) - overlap;
        if (d1 >= 0.0)
        {
            cp.push_back(v1);
        }
        if (d2 >= 0.0)
        {
            cp.push_back(v2);
        }
        if (d1 * d2 < 0.0)
        {

            utils::Vector2f e = v2 - v1;
            // compute the location along e
            float u = d1 / (d1 - d2);
            e *= u;
            e += v1;
            cp.push_back(e);
        }
        return cp;
    }

    std::vector<utils::Vector2f> inline clipEdges(CollisionFeature &ref_features, CollisionFeature &inc_features, utils::Vector2f n)
    {

        auto &ref_edge = ref_features.edge;
        auto &inc_edge = inc_features.edge;

        auto wtf_ref = std::abs(dot(ref_edge.t, n));
        auto wtf_inc = std::abs(dot(inc_edge.t, n));
        if (wtf_ref <= wtf_inc)
        {
        }
        else
        {
            std::swap(ref_features, inc_features);
        }

        utils::Vector2f ref_v = ref_edge.t;

        double o1 = dot(ref_v, ref_edge.from);
        // clip the incident edge by the first
        // vertex of the reference edge
        auto cp = clip(inc_edge.from, inc_edge.to(), ref_v, o1);
        auto cp_new = cp;
        // if we dont have 2 points left then fail
        if (cp.size() < 2)
        {
            return {};
        }

        double o2 = dot(ref_v, ref_edge.to());
        cp = clip(cp[0], cp[1], -ref_v, -o2);
        // if we dont have 2 points left then fail
        if (cp.size() < 2)
        {
            return {};
        }

        // get the reference edge normal
        utils::Vector2f refNorm = {-ref_v.y, ref_v.x};
        refNorm /= norm(refNorm);

        double max = dot(refNorm, ref_features.best_vertex);
        // make sure the final points are not past this maximum

        std::vector<float> depths(2);
        depths[0] = dot(refNorm, cp.at(0)) - max;
        depths[1] = dot(refNorm, cp.at(1)) - max;

        if (depths[0] < 0.0f)
        {
            cp.erase(cp.begin());
        }
        if (depths[1] < 0.0f)
        {
            cp.pop_back();
        }
        return cp;
    }

    void inline bounce(GameObject &obj1, GameObject &obj2, CollisionData c_data)
    {
        auto &rigid1 = obj1.getRigidBody();
        auto &rigid2 = obj2.getRigidBody();

        auto inertia1 = rigid1.inertia;
        auto inertia2 = rigid2.inertia;
        auto &angle_vel1 = rigid1.angle_vel;
        auto &angle_vel2 = rigid2.angle_vel;
        auto &mass1 = rigid1.mass;
        auto &mass2 = rigid2.mass;

        auto n = c_data.separation_axis;

        //! resolve interpenetration;
        float alpha = mass2 / (mass1 + mass2);
        obj1.move(-c_data.separation_axis * c_data.minimum_translation * alpha); //! separation axis always points from 1 to 2
        obj2.move(c_data.separation_axis * c_data.minimum_translation * (1 - alpha));

        auto cont_point = c_data.contact_point;

        auto v_rel = obj1.m_vel - obj2.m_vel;
        auto v_reln = dot(v_rel, n);

        float e = 1;
        float u_ab = 1. / mass1 + 1. / mass2;

        auto r_cont_coma = cont_point - obj1.getPosition();
        auto r_cont_comb = cont_point - obj2.getPosition();

        utils::Vector2f r_cont_coma_perp = {r_cont_coma.y, -r_cont_coma.x};
        utils::Vector2f r_cont_comb_perp = {r_cont_comb.y, -r_cont_comb.x};

        float ran = dot(r_cont_coma_perp, n);
        float rbn = dot(r_cont_comb_perp, n);

        float u_ab_rot = ran * ran / inertia1 + rbn * rbn / inertia2;

        float j_factor = -(1 + e) * v_reln / (u_ab + u_ab_rot);

        angle_vel1 += ran * j_factor / inertia1;
        angle_vel2 -= rbn * j_factor / inertia2;
        obj1.m_vel += j_factor / mass1 * n;
        obj2.m_vel -= j_factor / mass2 * n;
    }

} //! namespace collisions
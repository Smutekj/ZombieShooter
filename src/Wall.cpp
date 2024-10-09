#include "Wall.h"

#include "GameWorld.h"

Wall::Wall(TextureHolder &textures, utils::Vector2f from, utils::Vector2f to, EdgeId edge)
    : GameObject(&GameWorld::getWorld(), textures, ObjectType::Wall), m_cdt_edge(edge)
{
    m_collision_shape = std::make_unique<Polygon>(2);
    m_collision_shape->points.at(0) = {-0.5f, 0.f};
    m_collision_shape->points.at(1) = {0.5f, 0.f};
    // m_collision_shape->points.at(2) = {-0.5f, 0.f};
    auto dr = from - to;
    auto angle = utils::dir2angle(dr);
    auto length = dist(from, to);
    setPosition((from + to) / 2.f);
    setAngle(angle);
    setSize({length, 1.f});
}

void Wall::update(float dt)
{
}
void Wall::onCreation()
{
}
void Wall::onDestruction() {}
void Wall::draw(LayersHolder &layers)
{
    auto &canvas = layers.getCanvas("Wall");

    auto points = m_collision_shape->getPointsInWorld();
    assert(points.size() >= 2);
    auto v0 = points.at(0);
    auto v1 = points.at(1);
    canvas.drawLineBatched(v1, v0, 1.0, m_color);

    // auto norm = getNorm();
    // canvas.drawLineBatched(getPosition(), getPosition() + 5.f*norm, 1., {1,0,0,1});
}

void Wall::onCollisionWith(GameObject &obj, CollisionData &c_data)
{

    if (obj.getType() == ObjectType::Player || obj.getType() == ObjectType::Enemy)
    {
        auto points = m_collision_shape->getPointsInWorld();
        assert(points.size() >= 2);
        auto &v = obj.m_vel;
        auto pos = obj.getPosition();
        auto norm = getNorm();
        auto n_dot_v = dot(v, norm);
        auto is_left_of_wall = utils::orient(pos, points.at(0), points.at(1)) >= 0;
        if (is_left_of_wall)
        {
            if (n_dot_v >= 0 && m_constraints_motion)
            {
                v -= n_dot_v * norm; //! remove normal compotent to the wall (How to make this work with many constraints?)
                m_color = Color{20, 1, 0, 1};
            }
        }else
        {
            if (n_dot_v <= 0 && m_constraints_motion)
            {
                v -= n_dot_v * norm; //! remove normal compotent to the wall (How to make this work with many constraints?)
                m_color = Color{0.1, 1, 10, 1};
            }
        }
    }
    if (obj.getType() == ObjectType::Bullet)
    {
        // obj.kill();
    }
};
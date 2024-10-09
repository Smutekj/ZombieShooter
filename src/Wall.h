#include "GameObject.h"

struct EdgeId
{
    int tri_ind = -1;
    int ind_in_tri = 0;
};

class Wall : public GameObject
{

public:
    Wall() = default;
    explicit Wall(TextureHolder &textures, utils::Vector2f from, utils::Vector2f to, EdgeId edge);
    virtual ~Wall() override {}

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    utils::Vector2f getNorm()
    {

        auto points = m_collision_shape->getPointsInWorld();
        assert(points.size() >= 2);
        auto v0 = points.at(0);
        auto v1 = points.at(1);
        auto t = (v1 - v0);
        t /= norm(t);
        m_norm = utils::Vector2f{t.y, -t.x};
        return m_norm;
    }
private:

public:
    EdgeId m_cdt_edge;
    bool m_constraints_motion = true;
private:
    utils::Vector2f m_norm = {0, 0};
    Color m_color = {0, 1.f, 0, 1.f};
};

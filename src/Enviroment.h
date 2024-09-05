#pragma once

#include <Transform.h>
#include <View.h>
#include <VertexArray.h>
#include <Triangulation.h>
#include <Shader.h>
#include "Particles.h"
#include "GameObject.h"

class LayersHolder;

class EnviromentEffect : public GameObject
{

public:
    EnviromentEffect() = default;
    EnviromentEffect(TextureHolder &textures, ObjectType type);
    virtual ~EnviromentEffect() override {}

    virtual void update(float dt) = 0;
    virtual void onCreation() = 0;
    virtual void onDestruction() = 0;
    virtual void draw(LayersHolder &layers) = 0;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) final{};

private:
};

class FireEffect : public EnviromentEffect
{

public:
    FireEffect(TextureHolder &textures);
    virtual ~FireEffect() = default;

    void setEdgeColor(Color new_color);
    virtual void draw(LayersHolder &layers) override;
    virtual void update(float dt) override{}
    virtual void onCreation() override{}
    virtual void onDestruction() override{}

private:
    Particles m_fire;
    Particles m_smoke;

    Color m_smoke_color = {100, 100, 100, 1};
    Color m_smoke_edge_color = {0, 0, 0, 0};
    Color m_fire_color = {1000, 2, 0, 1};
    Color m_fire_edge_color = {1000, 2, 0, 0};
};

class MapGrid;

class Water : public EnviromentEffect
{

public:
    Water(ShaderHolder &shaders, LayersHolder &layers);
    virtual ~Water() = default;

    virtual void draw(LayersHolder &layers) override;
    virtual void update(float dt) override{}
    virtual void onCreation() override{}
    virtual void onDestruction() override{}
    
    void setColor(Color new_color);
    void readFromMap(cdt::Triangulation<cdt::Vector2i> &m_cdt, std::vector<int> &water_tri_inds);

    VertexArray m_water_verts;

private:
    Color m_water_color = {0.05, 0, 1, 1};
    Vec2 m_target_size;
};
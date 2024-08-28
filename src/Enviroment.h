#pragma once

#include <Transform.h>
#include <View.h>
#include <VertexArray.h>
#include <Triangulation.h>
#include <Shader.h>
#include "Particles.h"

class LayersHolder;

class EnviromentEffect : public Transform
{

public:
    virtual void draw(LayersHolder &layers, View view) = 0;

private:
    //! maybe have some generic effect-variables container here?
};

class FireEffect : public EnviromentEffect
{

public:
    FireEffect();
    virtual ~FireEffect() = default;

    void setEdgeColor(Color new_color);
    virtual void draw(LayersHolder &layers, View view) override;

private:
    Particles m_smoke;
    Particles m_fire;

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

    void setColor(Color new_color);
    virtual void draw(LayersHolder &layers, View view) override;
    void readFromMap(cdt::Triangulation<cdt::Vector2i> &m_cdt, std::vector<int> &water_tri_inds);

    VertexArray m_water_verts;
private:
    LayersHolder* m_layers;
    Color m_water_color = {0.05, 0, 1, 1};
};
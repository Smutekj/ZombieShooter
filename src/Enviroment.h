#pragma once

#include <Transform.h>
#include <View.h>
#include <VertexArray.h>
#include <Triangulation.h>
#include <Shader.h>
#include "Particles.h"
#include "GameObject.h"

struct LayersHolder;

class EnviromentEffect : public GameObject
{

public:
    EnviromentEffect() = default;
    EnviromentEffect(TextureHolder &textures);
    virtual ~EnviromentEffect() override {}

    virtual void update(float dt) = 0;
    virtual void onCreation() = 0;
    virtual void onDestruction() = 0;
    virtual void draw(LayersHolder &layers) = 0;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) final {};

    void setColor(const std::string &color_name, const Color &color)
    {
        m_colors[color_name] = color;
    }
    bool getColor(const std::string &color_name, Color &color)
    {
        if (m_colors.count(color_name) > 0)
        {
            color = m_colors.at(color_name);
            return true;
        }
        return false;
    }

protected:
    std::unordered_map<std::string, Color> m_colors;

private:
};

class FireEffect : public EnviromentEffect
{

public:
    FireEffect(ShaderHolder &shaders, TextureHolder &textures);
    virtual ~FireEffect() = default;

    void setEdgeColor(Color new_color);
    virtual void draw(LayersHolder &layers) override;
    virtual void update(float dt) override;
    virtual void onCreation() override {}
    virtual void onDestruction() override {}

private:
    Particles m_fire;
    Particles m_smoke;

    Color m_smoke_color = {100, 100, 100, 1};
    Color m_smoke_edge_color = {0, 0, 0, 0};
    Color m_fire_color = {1000, 2, 0, 1};
    Color m_fire_edge_color = {1000, 2, 0, 0};
};
class FloatingText : public EnviromentEffect
{

public:
    FloatingText(ShaderHolder &shaders, TextureHolder &textures);
    virtual ~FloatingText() = default;

    virtual void draw(LayersHolder &layers) override;
    virtual void update(float dt) override;
    virtual void onCreation() override {}
    virtual void onDestruction() override {}

private:
    Text m_text;

    float m_time = 0.f;
    float m_lifetime = 5.f;
};

class MapGrid;

class Water : public EnviromentEffect
{

public:
    Water(ShaderHolder &shaders, TextureHolder &textures);
    virtual ~Water() = default;

    virtual void draw(LayersHolder &layers) override;
    virtual void update(float dt) override {}
    virtual void onCreation() override {}
    virtual void onDestruction() override {}

    void setColor(Color new_color);
    void readFromMap(cdt::Triangulation<cdt::Vector2i> &m_cdt, std::vector<int> &water_tri_inds);

    VertexArray m_water_verts;

private:
    Color m_water_color = {0.05, 0, 1, 1};
    Vec2 m_target_size;
};
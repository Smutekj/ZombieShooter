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
    EnviromentEffect(TextureHolder &textures, const std::string& script_name);
    virtual ~EnviromentEffect() override {}

    virtual void update(float dt);
    virtual void onCreation() {};
    virtual void onDestruction() {};
    virtual void draw(LayersHolder &layers);
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) final {};

    void addParticles(const std::string& name, const std::string& layer_name = "Unit", int max_particle_count = 200);

    Particles* getParticlesP(const std::string& name) ;

    void doScript(const std::string &script_name);
    void loadFromScript();

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

    struct ParticlesDrawData{
        std::string layer_name = "Unit";
        Particles particles;
    };

    float m_lifetime = -1.f;
    float m_time = 0.f;

protected:
    std::string m_script_name = "testeffect.lua";
    std::string m_effect_id = "consecration";
    std::unordered_map<std::string, Color> m_colors;
    std::unordered_map<std::string, ParticlesDrawData> m_particles_holder;
    std::function<void(LayersHolder&)> m_drawer;
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

public:
    float m_lifetime = 5.f;

private:
    Text m_text;

    float m_time = 0.f;
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
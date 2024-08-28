
#pragma once

#include <Shader.h>
#include <Texture.h>
#include <Window.h>
#include <Renderer.h>
#include <FrameBuffer.h>
#include "DrawLayer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include <Utils/Vector2.h>

enum class UIWindowType
{
    DEBUG = 0,
    SHADERS,
    COUNT
};

struct DrawLayerUISlot
{

    DrawLayerUISlot(DrawLayer &layer)
        : m_layer(layer)
    {
    }

public:
    DrawLayer &m_layer;
    std::string m_selected_shader = "";
    std::string m_selected_uniform = "";
    std::string m_selected_texture = "";
};

struct ShaderSlot
{

    ShaderSlot(Shader &shader, std::string layer_name = "")
        : m_shader(shader), m_layer_name(layer_name)
    {
        
    }

public:
    Shader& m_shader;
    std::string m_layer_name = "";
    std::string m_selected_shader = "";
    std::string m_selected_uniform = "";
    std::string m_selected_texture = "";
};

constexpr int N_UI_WINDOWS = static_cast<int>(UIWindowType::COUNT);

class UIWindow
{

protected:
    std::string name;
    bool is_active = false;
    std::vector<std::unique_ptr<UIWindow>> children;

public:
    virtual void draw() = 0;
    virtual ~UIWindow() = 0;

    UIWindow(std::string name);

    const std::string &getName() const
    {
        return name;
    }
};

class Shader;

struct ColorData
{
    std::string uniform_name;
    glm::vec4 value = glm::vec4(0.5, 0.1, 0.5, 1.0);
};

struct UniformData
{
    std::string uniform_name;
    UniformType value;
};

struct BuildingLayer;

enum class TextureID
{
    ShaderOut1,
    ShaderOut2,
    Image,
};

class ShadersWindow : public UIWindow
{
    enum Data
    {
        COLOR1,
        COLOR2,
    };

public:
    ShadersWindow(TextureHolder &textures, LayersHolder &layers, Renderer& window);

    virtual ~ShadersWindow();
    virtual void draw() override;

private:
    void drawUniformValue(const char *uniform_nam, UniformType &value);
    void drawShaderSlot(ShaderSlot &slot);
    void refresh();

private:
    TextureHolder &m_textures;

    std::string m_selected_field = "";
    std::string m_output_image_name = "";

    std::vector<DrawLayerUISlot> m_slots;
    std::vector<ShaderSlot> m_shader_slots;
};

class FrameBuffer;
class LayersHolder;

class UI
{

    struct UIWindowData
    {
        std::unique_ptr<UIWindow> p_window;
        bool is_active = false;
        std::string name;
    };

    friend UIWindowType;

public:
    UI(Window &window, TextureHolder &textures, LayersHolder &layers,  Renderer& window_canvas);

    void showWindow();
    void draw(Window &window);
    void handleEvent(SDL_Event event);
    bool simulationRunning() const
    {
        return m_simulation_on;
    }

    bool resetSimulation()
    {
        if (m_reset)
        {
            m_reset = false;
            return true;
        }
        return false;
    }

    int getSimulationSlot()
    {
        return m_simulation_slot;
    }

    Color getParticleInitColor() const
    {
        return m_particle_init_color;
    }
    Color getParticleEndColor() const
    {
        return m_particle_end_color;
    }
    Color getBackgroundColor() const
    {
        return m_background_color;
    }
    Color getLightColor() const
    {
        return m_light_color;
    }

private:
    int m_simulation_slot = 0;
    bool m_simulation_on = false;
    bool m_reset = false;
    float value;
    utils::Vector2f m_mouse_coords_on_click;
    Color m_particle_init_color = {1, 100, 0, 1};
    Color m_particle_end_color = {1, 0, 0, 1};
    Color m_background_color = {0, 0, 0, 1};
    Color m_layer_background = {0, 0, 0, 0};
    Color m_light_color = {0, 0, 0, 0};
    std::unordered_map<UIWindowType, UIWindowData> m_window_data;

    LayersHolder &m_layers;
    std::string m_selected_layer = "Wall";

    std::vector<ShaderSlot> m_shaders_data;
    std::string m_command = "";
};

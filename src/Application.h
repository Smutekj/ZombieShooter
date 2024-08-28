#pragma once

#include <Window.h>
#include <Shader.h>
#include <Texture.h>
#include "ShaderUI.h"

#include <chrono>
#include <vector>
#include <numeric>
#include <unordered_map>
#include <memory>

#include <Rectangle.h>
#include <VertexArray.h>
#include <View.h>
#include <FrameBuffer.h>
#include <Renderer.h>
#include <Particles.h>
#include <Triangulation.h>

#include "Shadows/VisibilityField.h"
#include "MapGrid.h"
#include "Entities.h"
#include "GameWorld.h"
#include "DrawLayer.h"
#include "Enviroment.h"

void gameLoop(void *mainLoopArg);

class Application
{
public:
    Application(int width, int height);

    void run();
    void update(float dt);
    void handleInput();

private:
    void onKeyPress(SDL_Keycode key);
    void onKeyRelease(SDL_Keycode key);
    void onMouseButtonPress(SDL_MouseButtonEvent event);
    void onMouseButtonRelease(SDL_MouseButtonEvent event);
    void onWheelMove(SDL_MouseWheelEvent event);
    bool isKeyPressed(SDL_Scancode key)
    {
        auto *keystate = SDL_GetKeyboardState(NULL);
        return keystate[key];
    }

    void doBloom(Texture &source, Renderer &target);

    friend void gameLoop(void *);

    void fireProjectile(ProjectileTarget target, utils::Vector2f from);

private:
    LayersHolder m_layers;

    Window m_window;
    Renderer m_window_renderer;
    FrameBuffer m_scene_pixels;
    Renderer m_scene_canvas;

    TextureHolder m_textures;

    utils::Vector2f m_old_view_center;
    utils::Vector2f m_mouse_click_position;
    bool m_wheel_is_held = false;

    bool m_is_moving_right = false;
    bool m_is_moving_left = false;
    bool m_is_moving_up = false;
    bool m_is_moving_down = false;

    float m_time = 0.f;
    std::chrono::high_resolution_clock::time_point tic;

    std::vector<std::shared_ptr<EnviromentEffect>> m_enviroment;
    std::shared_ptr<Water> m_water;

    FrameBuffer test_pixels = {800, 600};
    Renderer test_canvas = {test_pixels};

    std::unique_ptr<UI> m_ui;

    cdt::Triangulation<cdt::Vector2i> m_cdt;
    VisionField m_vision;
    std::unique_ptr<MapGridDiagonal> m_map;

    std::shared_ptr<GameObject> p_player;
};

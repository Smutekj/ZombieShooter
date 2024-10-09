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
#include <Font.h>

#include "Shadows/VisibilityField.h"
#include "Map.h"
#include "Entities.h"
#include "GameWorld.h"
#include "DrawLayer.h"
#include "Enviroment.h"



namespace cdt{
    template class Triangulation<utils::Vector2i>;
}


struct Averager
{
    void addNumber(double num)
    {
        data.push_back(num);
        if(data.size() >= averaging_interval)
        {
            data.pop_front();
        }
        avg = std::accumulate(data.begin(), data.end(), 0.) / data.size();
    }
    std::deque<double> data;
    double avg = 0.;
    int averaging_interval = 60;
};

void gameLoop(void *mainLoopArg);


enum class WallType
{
    Door,
    Water,
    Fire
};

struct WallGameData
{
    int type = 0;
};

class Application
{
public:
    Application(int width, int height);

    void run();
    void update(float dt);
    void handleInput();

private:
    void drawUI(float dt);

    void onKeyPress(SDL_Keycode key);
    void onKeyRelease(SDL_Keycode key);
    void onMouseButtonPress(SDL_MouseButtonEvent event);
    void onMouseButtonRelease(SDL_MouseButtonEvent event);
    void onWindowResize(SDL_WindowEvent event);
    void onWheelMove(SDL_MouseWheelEvent event);
    
    void moveView(utils::Vector2f dr, Renderer &target);
    static bool isKeyPressed(SDL_Scancode key)
    {
        auto *keystate = SDL_GetKeyboardState(NULL);
        return keystate[key];
    }

    void initializeLayers();


    friend void gameLoop(void *);

    void fireProjectile(ProjectileTarget target, utils::Vector2f from);
    void changeShield();

    void selectInWorld(const utils::Vector2f& left_select, const utils::Vector2f& right_select);

private:
    SurfaceManager m_surfaces;
    LayersHolder m_layers;

    Window m_window;
    Renderer m_window_renderer;
    FrameBuffer m_scene_pixels;
    Renderer m_scene_canvas;

    TextureHolder m_textures;

    utils::Vector2f m_old_view_center;
    utils::Vector2f m_mouse_click_position;
    utils::Vector2f m_selection_click_pos ;
    utils::Vector2f m_old_player_dir ;
    float m_old_angle ;
    bool m_is_selecting = false;
    bool m_wheel_is_held = false;

    bool m_is_moving_right = false;
    bool m_is_moving_left = false;
    bool m_is_moving_up = false;
    bool m_is_moving_down = false;
    
    bool m_is_turning_right = false;
    bool m_is_turning_left = false;
    bool m_is_sprinting = false;
    bool m_is_turning = false;

    bool m_is_selecting_wall = false;

    float m_time = 0.f;
    std::chrono::high_resolution_clock::time_point tic;

    FrameBuffer test_pixels = {800, 600};
    Renderer test_canvas = {test_pixels};

    std::unique_ptr<UI> m_ui;

    std::unique_ptr<MapGridDiagonal> m_map;

    std::shared_ptr<PlayerEntity> p_player;

    std::shared_ptr<Font> m_font;

    Averager m_avg_frame_time;



};

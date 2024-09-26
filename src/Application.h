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
#include "MapGrid.h"
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

std::vector<int> inline findConnectedTriInds(cdt::Triangulation<cdt::Vector2i> &cdt, int start_tri_ind)
{
    std::vector<int> connected_inds;
    auto &triangles = cdt.m_triangles;
    auto n_triangles = triangles.size();
    std::unordered_set<int> visited(n_triangles);
    std::queue<int> to_visit;
    to_visit.push(start_tri_ind);
    while (!to_visit.empty())
    {
        auto curr = to_visit.front();
        auto &curr_tri = triangles.at(curr);
        to_visit.pop();
        visited.insert(curr);
        connected_inds.push_back(curr);
        for (int i = 0; i < 3; ++i)
        {
            auto neighbour = curr_tri.neighbours[i];
            if (!visited.contains(neighbour) && !curr_tri.is_constrained.at(i))
            {
                to_visit.push(neighbour);
            }
        }
    }
    return connected_inds;
}

enum class SurfaceType
{
    Water,
    Lava,
    Poison,
    Gay
};

using RegionId = std::size_t;
struct SurfaceData
{
    SurfaceType type;
    RegionId region = 0;
};

class SurfaceManager
{

public:
    bool readFromMap(MapGridDiagonal &map)
    {
        auto &world = GameWorld::getWorld();
        std::string wall_name = "Wall0";
        int i = 0;
        while (world.kill(wall_name)) //! destroy all walls
        {
            i++;
            wall_name = "Wall" + std::to_string(i);
        }

        auto edges = map.extractEdges();
        std::vector<cdt::EdgeVInd> edge_inds;

        i = 0;
        for (auto &e : edges)
        {
            auto v0 = utils::Vector2f{e.from.x, e.from.y};
            auto v1 = utils::Vector2f{e.to().x, e.to().y};
            wall_name = "Wall" + std::to_string(i);
            auto obj = GameWorld::getWorld().addObject<Wall>(wall_name, v0, v1);
            i++;
        }
        return true;
    }

    void ad(cdt::Triangulation<cdt::Vector2i> &cdt, utils::Vector2f pos)
    {
        m_tri2surface.resize(cdt.m_triangles.size());

        auto tri_ind = cdt.findTriangle({pos.x, pos.y});
        auto connected_inds = findConnectedTriInds(cdt, tri_ind);

        auto region = m_tri2surface.at(tri_ind).region;
        auto type = m_tri2surface.at(tri_ind).type;
        if (m_region2tri_inds.count(region))
        {
        }
        if (m_surface2regions.count(type))
        {
        }
        else
        {
        }

        if (tri_ind >= m_tri2surface.size())
        {
            //! upgrade memory
        }
    }

public:
    using RegionMap = std::unordered_map<RegionId, std::vector<cdt::TriInd>>;
    using SurfaceMap = std::unordered_map<SurfaceType, std::vector<RegionId>>;

    RegionMap m_region2tri_inds;
    SurfaceMap m_surface2regions;
    std::vector<SurfaceData> m_tri2surface;
};

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
    bool m_is_selecting = false;
    bool m_wheel_is_held = false;

    bool m_is_moving_right = false;
    bool m_is_moving_left = false;
    bool m_is_moving_up = false;
    bool m_is_moving_down = false;
    
    bool m_is_sprinting = false;

    float m_time = 0.f;
    std::chrono::high_resolution_clock::time_point tic;

    FrameBuffer test_pixels = {800, 600};
    Renderer test_canvas = {test_pixels};

    std::unique_ptr<UI> m_ui;

    std::unique_ptr<MapGridDiagonal> m_map;

    std::shared_ptr<GameObject> p_player;

    std::shared_ptr<Font> m_font;

    Averager m_avg_frame_time;
};

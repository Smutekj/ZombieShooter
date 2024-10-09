#include "Application.h"
#include "PostEffects.h"
#include "DrawLayer.h"
#include "LuaWrapper.h"
#include "Wall.h"

#include <IncludesGl.h>
#include <Utils/RandomTools.h>
#include <Utils/IO.h>
#include <LuaBridge/LuaBridge.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include <time.h>
#include <chrono>
#include <queue>
#include <filesystem>

template <class UniformType>
struct ShaderUniform
{
    std::string name;
    UniformType value;
};

void drawLine(Renderer &canvas, utils::Vector2f from, utils::Vector2f to, float thickness, Color color = {1, 0, 0, 1})
{
    canvas.drawLineBatched({from.x, from.y}, {to.x, to.y}, thickness, color);
}

void drawGrid(Renderer &canvas, utils::Vector2i grid_size)
{
    for (float ix = 0; ix < grid_size.x; ++ix)
    {
        canvas.drawLine(Vec2{ix, 0}, Vec2{ix, (float)grid_size.y}, 2.69, Color{10, 0, 0, 1});
    }

    for (float iy = 0; iy < grid_size.y; ++iy)
    {
        canvas.drawLine(Vec2{0, iy}, Vec2{iy, (float)grid_size.x}, 2.69, Color{10, 0, 0, 1});
    }
}

template <class T>
void drawTriangles(const cdt::Triangulation<T> &cdt, Renderer &canvas, Color wall_color)
{
    for (auto &tri : cdt.m_triangles)
    {
        utils::Vector2f v1(tri.verts[0].x, tri.verts[0].y);
        utils::Vector2f v2(tri.verts[1].x, tri.verts[1].y);
        utils::Vector2f v3(tri.verts[2].x, tri.verts[2].y);
        if (tri.is_constrained[0])
        {
            drawLine(canvas, v1, v2, 1.0, wall_color);
        }
        if (tri.is_constrained[1])
        {
            drawLine(canvas, v2, v3, 1.0, wall_color);
        }
        if (tri.is_constrained[2])
        {
            drawLine(canvas, v3, v1, 1.0, wall_color);
        }
    }
}

template <class... UniformType>
void setUniforms(Shader &program, ShaderUniform<UniformType> &...values)
{
    (program.setUniform(values.name, values.value), ...);
}

void drawProgramToTexture(Sprite &rect, Renderer &target, std::string program)
{
    target.clear({1, 1, 1, 1});
    target.drawSprite(rect, program, DrawType::Dynamic);
    target.drawAll();
}

void updateTriangulation(cdt::Triangulation<cdt::Vector2i> &cdt, MapGridDiagonal &map, SurfaceManager &surfaces)
{
    cdt.reset();
    map.extractBoundaries();
    auto edges = map.extractEdges();
    std::vector<cdt::EdgeVInd> edge_inds;

    int dx = map::MAP_SIZE_X / map::MAP_GRID_CELLS_X;
    int dy = map::MAP_SIZE_Y / map::MAP_GRID_CELLS_Y;

    //! insert points around the map perimeter to prevent large
    utils::Vector2i prev_point1 = {dx, dy};
    utils::Vector2i prev_point2 = {dx, (map.m_cell_count.y - 1) * dy};
    for (int ix = 3; ix < map.m_cell_count.x - 1; ix += 2)
    {
        utils::Vector2i point1 = {ix * dx, dy};
        utils::Vector2i point2 = {ix * dx, (map.m_cell_count.y - 1) * dy};
        edges.push_back({prev_point1, point1});
        edges.push_back({prev_point2, point2});
        prev_point1 = point1;
        prev_point2 = point2;
    }
    prev_point1 = {dx, dy};
    prev_point2 = {(map.m_cell_count.x - 1) * dx, dy};
    for (int iy = 3; iy < map.m_cell_count.y - 1; iy += 2)
    {
        utils::Vector2i point1 = {dx, iy * dy};
        utils::Vector2i point2 = {(map.m_cell_count.x - 1) * dx, iy * dy};
        edges.push_back({prev_point1, point1});
        edges.push_back({prev_point2, point2});
        prev_point1 = point1;
        prev_point2 = point2;
    }

    for (auto &e : edges)
    {
        cdt::EdgeVInd e_ind;
        auto v_ind1 = cdt.insertVertexAndGetData({e.from.x, e.from.y}).overlapping_vertex;
        e_ind.from = (v_ind1 == -1 ? cdt.m_vertices.size() - 1 : v_ind1);

        auto v_ind2 = cdt.insertVertexAndGetData({e.to().x, e.to().y}).overlapping_vertex;
        e_ind.to = (v_ind2 == -1 ? cdt.m_vertices.size() - 1 : v_ind2);

        edge_inds.push_back(e_ind);
    }
    for (auto &e : edge_inds)
    {
        cdt.insertConstraint(e);
    }

    if (!surfaces.readFromMap(edge_inds, edges, cdt))
    {
        throw std::runtime_error("Map is fucked :(");
    }
    //! update pathfinder
    GameWorld::getWorld().getPathFinder().update();

    //! update vision fields
    auto player = GameWorld::getWorld().get<PlayerEntity>("Player");
    player->m_vision.onTriangulationChange();
}

void Application::initializeLayers()
{
    auto width = m_window.getSize().x;
    auto height = m_window.getSize().y;

    TextureOptions options;
    options.wrap_x = TexWrapParam::ClampEdge;
    options.wrap_y = TexWrapParam::ClampEdge;

    TextureOptions text_options;
    text_options.data_type = TextureDataTypes::UByte;
    text_options.format = TextureFormat::RGBA;
    text_options.internal_format = TextureFormat::RGBA;

    auto &text_layer = m_layers.addLayer("Text", 100, text_options);
    text_layer.m_canvas.addShader("Text", "basicinstanced.vert", "textBorder.frag");

    auto &unit_layer = m_layers.addLayer("Unit", 3, options);
    // unit_layer.addEffect(std::make_unique<Bloom2>(width, height));
    unit_layer.m_canvas.addShader("Shiny", "basicinstanced.vert", "shiny.frag");
    unit_layer.m_canvas.addShader("lightning", "basicinstanced.vert", "lightning.frag");

    auto &smoke_layer = m_layers.addLayer("Smoke", 4, options);
    // smoke_layer.addEffect(std::make_unique<BloomSmoke>(width, height));

    auto &fire_layer = m_layers.addLayer("Fire", 6, options);
    fire_layer.addEffect(std::make_unique<Bloom2>(width, height, options));

    auto &wall_layer = m_layers.addLayer("Wall", 5000, options);
    wall_layer.addEffect(std::make_unique<Bloom3>(width, height, options));

    auto &light_layer = m_layers.addLayer("Light", 150, options);
    light_layer.m_canvas.m_blend_factors = {BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha};
    light_layer.addEffect(std::make_unique<SmoothLight>(width, height));
    light_layer.addEffect(std::make_unique<LightCombine>(width, height));
    light_layer.m_canvas.addShader("VisionLight", "basictex.vert", "fullpassLight.frag");
    light_layer.m_canvas.addShader("combineBloomBetter", "basicinstanced.vert", "combineBloomBetter.frag");

    auto &water_layer = m_layers.addLayer("Water", 0, options);

    auto &ui_layer = m_layers.addLayer("UI", 1000, text_options);
    // water_layer.addEffect(std::make_unique<WaterEffect>(width, height));
    //
}

Application::Application(int width, int height) : m_window(width, height),
                                                  m_window_renderer(m_window),
                                                  m_scene_pixels(width, height),
                                                  m_scene_canvas(m_scene_pixels)
{
    using namespace map;
    GameWorld::getWorld().getPathFinder().m_surfaces = &m_surfaces;

    initializeLayers();

    m_font = std::make_shared<Font>("arial.ttf");

    m_map = std::make_unique<MapGridDiagonal>(utils::Vector2i{MAP_SIZE_X, MAP_SIZE_Y},
                                              utils::Vector2i{MAP_GRID_CELLS_X, MAP_GRID_CELLS_Y});
    auto &world = GameWorld::getWorld();
    for (int i = 0; i < 69; ++i)
    {
        auto rand_pos = randomPosInBox({5, 5}, {0.95 * MAP_SIZE_X, 0.95 * MAP_SIZE_Y});
        auto map_cell = m_map->coordToCell(rand_pos);
        int rx = 2 + rand() % 4;
        int ry = 2 + rand() % 4;
        m_map->changeTiles(MapGridDiagonal::Tile::Wall, {m_map->cellCoordX(map_cell), m_map->cellCoordY(map_cell)}, {rx, ry});
    }

    p_player = world.addObject<PlayerEntity>("Player");
    p_player->setPosition({500, 500});
    p_player->setAngle(90);
    world.addObject(ObjectType::Orbiter, "Shield", world.getIdOf("Player"));

    updateTriangulation(world.getTriangulation(), *m_map, m_surfaces);

    m_window_renderer.addShader("circle", "basicinstanced.vert", "circle.frag");
    m_window_renderer.addShader("Shiny", "basicinstanced.vert", "shiny.frag");
    m_window_renderer.addShader("Water", "basictex.vert", "test.frag");
    m_window_renderer.addShader("Instanced", "basicinstanced.vert", "texture.frag");
    m_window_renderer.addShader("LastPass", "basicinstanced.vert", "lastPass.frag");
    m_window_renderer.addShader("VertexArrayDefault", "basictex.vert", "fullpass.frag");
    m_window_renderer.addShader("Text", "basicinstanced.vert", "textBorder.frag");
    m_scene_canvas.addShader("Instanced", "basicinstanced.vert", "texture.frag");
    m_scene_canvas.addShader("gaussHoriz", "basicinstanced.vert", "gaussHoriz.frag");
    m_scene_canvas.addShader("VertexArrayDefault", "basictex.vert", "fullpass.frag");
    m_scene_canvas.addShader("combineBloom", "basicinstanced.vert", "combineBloom.frag");
    m_scene_canvas.addShader("combineBloomBetter", "basicinstanced.vert", "combineBloomBetter.frag");
    m_scene_canvas.addShader("combineSmoke", "basicinstanced.vert", "combineSmoke.frag");
    m_scene_canvas.addShader("combineEdges", "basicinstanced.vert", "combineEdges.frag");

    // m_water = std::make_shared<Water>(m_window_renderer.getShaders(), m_layers);
    auto &water = world.addVisualEffect<Water>("Water");
    // auto effect = world.addEffect("Fire", "TestFire", -1);
    // effect->setPosition(p_player->getPosition());
    std::filesystem::path texture_path = "../Resources/Textures/";
    auto texture_filenames = extractNamesInDirectory(texture_path, ".png");
    for (auto &texture_filename : texture_filenames)
    {
        auto pos_right = texture_filename.find_last_of('.');
        std::string texture_name = texture_filename.substr(0, pos_right);
        m_textures.add(texture_name, texture_path.string() + texture_filename);
    }

    //! set view and add it to renderers
    m_window_renderer.m_view.setSize(m_window.getSize());
    m_window_renderer.m_view.setCenter(m_window.getSize() / 2);

    // m_layers.activate("Light");
    // m_layers.activate("Smoke");
    // m_layers.activate("Fire");
    // m_layers.activate("Unit");
    // m_layers.activate("UI");
    m_ui = std::make_unique<UI>(m_window, m_textures, m_layers, m_window_renderer);
}

void Application::run()
{

#ifdef __EMSCRIPTEN__
    int fps = 0; // Use browser's requestAnimationFrame
    emscripten_set_main_loop_arg(gameLoop, (void *)this, fps, true);
#else
    while (!m_window.shouldClose())
        gameLoop((void *)this);
#endif
}

void Application::handleInput()
{

    auto &imgui_io = ImGui::GetIO();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        m_ui->handleEvent(event);
        if (imgui_io.WantCaptureMouse)
        {
            continue;
        }
        switch (event.type)
        {
        case SDL_KEYDOWN:
            onKeyPress(event.key.keysym.sym);
            break;
        case SDL_KEYUP:
            onKeyRelease(event.key.keysym.sym);
            break;
        case SDL_MOUSEWHEEL:
            onWheelMove(event.wheel);
            break;
        case SDL_MOUSEBUTTONDOWN:
            onMouseButtonPress(event.button);
            break;
        case SDL_MOUSEBUTTONUP:
            onMouseButtonRelease(event.button);
            break;
        case SDL_WINDOWEVENT:
        {
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                onWindowResize(event.window);
            }
            break;
        }
        default:
            break;
        }
    }
}

void Application::onKeyPress(SDL_Keycode key)
{
    auto mouse_pos = m_window_renderer.getMouseInWorld();

    switch (key)
    {
    case SDLK_a:
        m_is_turning_left = true;
        break;
    case SDLK_e:
        m_is_moving_right = true;
        break;
    case SDLK_d:
        m_is_turning_right = true;
        break;
    case SDLK_w:
        m_is_moving_up = true;
        break;
    case SDLK_q:
        m_is_moving_left = true;
        break;
    case SDLK_s:
        m_is_moving_down = true;
        break;
    case SDLK_LSHIFT:
        m_is_sprinting = true;
        break;
    case SDLK_c:
        changeShield();
        break;
    case SDLK_SPACE:
        fireProjectile(mouse_pos, p_player->getPosition());
        break;
    }
}

void Application::onWheelMove(SDL_MouseWheelEvent event)
{

    if (event.preciseY < 0)
    {
        m_window_renderer.m_view.zoom(0.95f);
    }
    else if (event.preciseY > 0)
    {

        m_window_renderer.m_view.zoom(1. / 0.95f);
    }
}

void Application::onMouseButtonPress(SDL_MouseButtonEvent event)
{
    if (event.button == SDL_BUTTON_MIDDLE)
    {
        m_wheel_is_held = true;
        m_old_view_center = m_window_renderer.m_view.getCenter();
        m_mouse_click_position = m_window_renderer.getMouseInWorld();
    }
    else if (event.button == SDL_BUTTON_LEFT)
    {
        if (!m_is_selecting)
        {
            m_selection_click_pos = m_window_renderer.getMouseInWorld();
            m_is_selecting = true;
        }
    }
    else if (event.button == SDL_BUTTON_RIGHT)
    {
        m_old_player_dir = m_window_renderer.getMouseInWorld() - p_player->getPosition();
        m_old_angle = p_player->getAngle();
        m_is_turning = true;
    }
}
void Application::onWindowResize(SDL_WindowEvent event)
{
    m_window.setSize(event.data1, event.data2);
}

void Application::selectInWorld(const utils::Vector2f &left_select, const utils::Vector2f &right_select)
{
    auto &world = GameWorld::getWorld();

    utils::Vector2f lower_left = {std::min(left_select.x, right_select.x), std::min(left_select.y, right_select.y)};
    utils::Vector2f upper_right = {std::max(left_select.x, right_select.x), std::max(left_select.y, right_select.y)};
    AABB selection_rect = {lower_left, upper_right};

    auto selected_enemies = world.getCollider().findNearestObjects(ObjectType::Enemy, selection_rect);
    if (!selected_enemies.empty())
    {
        p_player->target_enemy = static_cast<Enemy *>(selected_enemies[0]);
    }
}

void Application::onMouseButtonRelease(SDL_MouseButtonEvent event)
{
    auto mouse_pos = m_window_renderer.getMouseInWorld();
    if (event.button == SDL_BUTTON_MIDDLE)
    {
        auto mouse_pos2 = m_window_renderer.getMouseInWorld();
        m_wheel_is_held = false;
    }
    else if (event.button == SDL_BUTTON_LEFT)
    {
        m_is_selecting = false;
        if (m_is_selecting_wall)
        {
            auto selected_walls = selectWalls(m_selection_click_pos, mouse_pos);
            for (auto wall : selected_walls)
            {
                wall->m_constraints_motion = !wall->m_constraints_motion;
                p_player->m_vision.toggleVisibility(wall->m_cdt_edge.tri_ind, wall->m_cdt_edge.ind_in_tri);
            }
        }
        else
        {
            selectInWorld(m_selection_click_pos, mouse_pos);
        }
    }
    else if (event.button == SDL_BUTTON_RIGHT)
    {
        m_is_turning = false;
    }
    if (isKeyPressed(SDL_SCANCODE_LCTRL) && event.button == SDL_BUTTON_RIGHT)
    {
        auto map_cell = m_map->coordToCell(mouse_pos.x, mouse_pos.y);
        m_map->changeTiles(MapGridDiagonal::Tile::Wall, {m_map->cellCoordX(map_cell), m_map->cellCoordY(map_cell)}, {5, 5});
        auto &cdt = GameWorld::getWorld().getTriangulation();
        updateTriangulation(cdt, *m_map, m_surfaces);
        auto &pf = GameWorld::getWorld().getPathFinder();
        pf.update();
    }
    if (isKeyPressed(SDL_SCANCODE_LSHIFT) && event.button == SDL_BUTTON_RIGHT)
    {
        auto &cdt = GameWorld::getWorld().getTriangulation();
        auto tri_ind = cdt.findTriangle({mouse_pos.x, mouse_pos.y});
        auto connected_inds = findConnectedTriInds(cdt, tri_ind);
        auto water = GameWorld::getWorld().get("Water");

        if (water)
        {
            static_cast<Water &>(*water).readFromMap(cdt, connected_inds);
            m_surfaces.changeSurface(cdt, mouse_pos, SurfaceType::Water);
        }
    }

    auto lua = LuaWrapper::getSingleton();
    auto script_status = LuaWrapper::loadScript("UI/onAction.lua");
    if (script_status != LuaScriptStatus::Broken)
    {
        try
        {
            auto ability = luabridge::getGlobal(lua->m_lua_state, "DoAbility");
            ability(mouse_pos, utils::Vector2f{0, 0});
        }
        catch (std::exception &e)
        {
            std::cout << "... " << e.what() << "\n";
        }
    }
}

void Application::onKeyRelease(SDL_Keycode key)
{
    auto mouse_pos = m_window_renderer.getMouseInWorld();

    switch (key)
    {
    case SDLK_a:
        m_is_turning_left = false;
        break;
    case SDLK_e:
        m_is_moving_right = false;
        break;
    case SDLK_d:
        m_is_turning_right = false;
        break;
    case SDLK_w:
        m_is_moving_up = false;
        break;
    case SDLK_q:
        m_is_moving_left = false;
        break;
    case SDLK_s:
        m_is_moving_down = false;
        break;
    case SDLK_t:
        m_is_selecting_wall = !m_is_selecting_wall;
        break;
    case SDLK_LSHIFT:
        m_is_sprinting = false;
        break;
    case SDLK_ESCAPE:
        m_window.close();
        break;
    }
}

enum class SpellId
{
    FireBolt,
    FrostBolt,
    LightningBolt,
};

struct Spell
{

    std::string m_script_effect_name = "";
    SpellId id = SpellId::FireBolt;
};

void Application::fireProjectile(ProjectileTarget target, utils::Vector2f from)
{
    auto &world = GameWorld::getWorld();

    // auto status = LuaWrapper::loadScript("onAction.lua");
    // if(status != LuaScriptStatus::Broken)
    // {
    //     p_player->m_combat_state = CombatState::Casting;
    //     p_player->m_cast_time = 0.f;
    // }
    // else
    // {
    //     return;
    // }

    if (p_player->target_enemy)
    {
        if (p_player->target_enemy->isDead())
        {
            p_player->target_enemy = nullptr;
            p_player->setTarget(nullptr);
            return;
        }
        auto &projectile = static_cast<Projectile &>(*world.addObject(ObjectType::Bullet, "Projectile"));
        projectile.setScriptName("frostbolt");
        projectile.setPosition(from);
        projectile.setSize(50.f);
        projectile.setTarget(p_player->target_enemy);
        p_player->setTarget(p_player->target_enemy);
    }
}

void Application::changeShield()
{
    auto &world = GameWorld::getWorld();
    world.destroyObject(world.getIdOf("Shield"));
    world.update(0);
    auto shield = world.addObject(ObjectType::Orbiter, "Shield", world.getIdOf("Player"));
}

View m_default_view;

void Application::moveView(utils::Vector2f dr, Renderer &target)
{
    auto view = target.m_view;
    auto old_view_center = view.getCenter();
    auto new_view_center = old_view_center - (dr);
    view.setCenter(new_view_center.x, new_view_center.y);

    auto player = GameWorld::getWorld().get<PlayerEntity>("Player");
    if (!player)
    {
        return;
    }
    //! look from higher distance when boosting
    // float booster_ratio = norm(player->m_vel) / player->getMaxSpeed();
    // view.setSize(m_default_view.getScale() * (1 + booster_ratio / 3.f));

    auto threshold = view.getScale() / 2.f - view.getScale() / 3.f;
    threshold *= 0.f;
    auto dx = player->getPosition().x - view.getCenter().x;
    auto dy = player->getPosition().y - view.getCenter().y;
    auto view_max = view.getCenter() + view.getScale() / 2.f;
    auto view_min = view.getCenter() - view.getScale() / 2.f;

    //! move view when approaching sides
    if (dx > threshold.x && view_max.x < m_map->getSizeX())
    {
        view.setCenter(view.getCenter() + utils::Vector2f{dx - threshold.x, 0});
    }
    else if (dx < -threshold.x && view_min.x > 0)
    {
        view.setCenter(view.getCenter() + utils::Vector2f{dx + threshold.x, 0});
    }
    if (dy > threshold.y && view_max.y < m_map->getSizeY())
    {
        view.setCenter(view.getCenter() + utils::Vector2f{0, dy - threshold.y});
    }
    else if (dy < -threshold.y && view_min.y > 0)
    {
        view.setCenter(view.getCenter() + utils::Vector2f{0, dy + threshold.y});
    }
    target.m_view = view;
}

float inline dir2angle(const utils::Vector2f &dir)
{
    return 180.f / std::numbers::pi_v<float> * std::atan2(dir.y, dir.x);
}

void Application::update(float dt = 0.016f)
{

    if (m_wheel_is_held)
    {
        auto mouse_coords = m_window_renderer.getMouseInWorld();
        auto dr = mouse_coords - m_mouse_click_position;
        if (utils::norm(dr) > 0.5f)
        {
            moveView(dr, m_window_renderer);
        }
    }
    else
    {
        moveView({0, 0}, m_window_renderer);
    }

    auto &world = GameWorld::getWorld();
    auto p_player = world.get<PlayerEntity>("Player");
    if (p_player)
    {

        auto dir = utils::angle2dir(p_player->getAngle());
        auto vel = p_player->getMaxSpeed() * dir;
        // p_player->m_vel = p_player->getMaxSpeed() * ((int)m_is_moving_right - (int)m_is_moving_left);
        p_player->m_vel = vel * ((int)m_is_moving_up - (int)m_is_moving_down);
        if (m_is_moving_left || m_is_moving_right)
        {
            utils::Vector2f n_dir = {dir.y, -dir.x};
            p_player->m_vel += n_dir * p_player->getMaxSpeed() * ((int)m_is_moving_right - (int)m_is_moving_left);
        }
        if (m_is_turning_right)
        {
            p_player->setAngle(p_player->getAngle() - 5);
        }
        if (m_is_turning_left)
        {
            p_player->setAngle(p_player->getAngle() + 5);
        }
        int a = m_is_moving_down + m_is_moving_left + m_is_moving_up + m_is_moving_right;
        if (a >= 2)
        {
            p_player->m_vel /= std::sqrt(2.f);
        }
        if (m_is_sprinting)
        {
            p_player->m_vel *= 3.f;
        }
        if (m_is_turning)
        {
            auto mouse_pos = m_window_renderer.getMouseInWorld();
            auto dr_new = mouse_pos - p_player->getPosition();
            auto dr_prev = m_old_player_dir;
            dr_new /= norm(dr_new);
            dr_prev /= norm(dr_prev);
            auto new_dir = utils::angle2dir(m_old_angle);
            if (!utils::approx_equal_zero(norm(dr_new - dr_prev)))
            {
                auto delta_vec = 0.5 * (dr_new - dr_prev) / norm(dr_new - dr_prev);
                new_dir += delta_vec;
                // m_turning_pivot += delta_vec;
            }
            else
            {
            }
            float new_angle = utils::dir2angle(dr_new);
            p_player->setAngle(new_angle);
        }
    }
    world.update(dt);
    m_time += dt;
    //! set time in shaders
    Shader::m_time = m_time;

    auto &unit_canvas = m_layers.getCanvas("Unit");
    auto &wall_canvas = m_layers.getCanvas("Wall");

    auto mouse_coords = m_window_renderer.getMouseInWorld();
    auto wall_color = m_ui->getBackgroundColor();
    // drawTriangles(world.getTriangulation(), wall_canvas, wall_color);

    //! draw world
    m_layers.clearAllLayers();
    world.draw(m_layers);

    //! clear and draw into scene
    m_scene_canvas.clear({0, 0, 0, 0});
    //! draw background
    utils::Vector2f map_size = {m_map->m_cell_count.x * m_map->m_cell_size.x,
                                m_map->m_cell_count.y * m_map->m_cell_size.y};
    auto old_view = m_scene_canvas.m_view;
    if (auto text = m_textures.get("grass"))
    {
        Sprite background(*m_textures.get("grass"));
        background.m_color = ColorByte{255, 255, 255, 0};

        background.setPosition(map_size / 2.f);
        background.setScale(map_size / 2.f);
        m_scene_canvas.m_view = m_window_renderer.m_view;
        m_scene_canvas.drawSprite(background, "Instanced", DrawType::Dynamic);
        m_scene_canvas.drawAll();
        m_scene_canvas.m_view = old_view;
        m_layers.draw(m_scene_canvas, m_window_renderer.m_view);
    }

    // //! draw everything to a window quad
    m_window.clear({0, 0, 0, 0});
    auto scene_size = m_scene_pixels.getSize();

    old_view = m_window_renderer.m_view;
    Sprite screen_sprite(m_scene_pixels.getTexture());
    screen_sprite.setPosition(scene_size / 2.f);
    screen_sprite.setScale(scene_size / 2.f);
    m_window_renderer.m_view.setCenter(screen_sprite.getPosition());
    m_window_renderer.m_view.setSize(scene_size);
    m_window_renderer.drawSprite(screen_sprite, "LastPass", DrawType::Dynamic);
    auto old_factors = m_window_renderer.m_blend_factors;
    m_window_renderer.m_blend_factors = {BlendFactor::One, BlendFactor::OneMinusSrcAlpha};
    m_window_renderer.drawAll();
    m_window_renderer.m_blend_factors = old_factors;
    m_window_renderer.m_view = old_view;

    drawUI(dt);
}

void Application::drawUI(float dt)
{

    auto old_view = m_window_renderer.m_view;

    Text text;
    m_window_renderer.m_view.setCenter(m_window.getSize() / 2);
    m_window_renderer.m_view.setSize(m_window.getSize());
    text.setPosition(100, 0);
    text.setScale(1, 1);
    text.setFont(m_font.get());
    text.setColor({255, 255, 255, 255});
    std::string text_test = "Frame time: ... " + std::to_string(m_avg_frame_time.avg);
    text.setText(text_test);
    m_window_renderer.drawText(text, "Text", DrawType::Dynamic);

    Sprite health_bar(*m_textures.get("bomb"));
    health_bar.m_texture_handles[0] = 0;

    // float x_scale = p_player->m_health / (double)p_player->m_max_health;
    // utils::Vector2f window_size = m_window.getSize();
    // health_bar.setPosition(0.2 * window_size.x, 0.95 * window_size.y);
    // health_bar.setScale(100, 15);
    // m_window_renderer.drawSprite(health_bar, "healthBar", DrawType::Dynamic);
    // m_window_renderer.getShader("healthBar").getVariables().uniforms["u_health_percentage"] = x_scale;
    // m_window_renderer.drawAll();

    //! run lua script for UI
    if (LuaWrapper::loadScript("UI/playerUI.lua") != LuaScriptStatus::Broken)
    {
        auto drawer = luabridge::getGlobal(LuaWrapper::getSingleton()->m_lua_state, "DrawUI");
        if (drawer.isFunction())
        {
            try
            {
                drawer(p_player.get(), &m_window_renderer, &m_layers);
            }
            catch (std::exception &e)
            {
                std::cout << "ERROR IN DrawUI: " << e.what() << "\n";
            }
        }
    }
    m_window_renderer.drawAll();
    m_window_renderer.m_view = old_view;
    m_ui->draw(m_window);
}

void inline gameLoop(void *mainLoopArg)
{
#ifdef __EMSCRIPTEN__
    // emscripten_trace_record_frame_start();
#endif
    auto tic = clock();
    // auto tic = std::chrono::high_resolution_clock::now();
    Application *p_app = (Application *)mainLoopArg;

    p_app->update(0.01666666);
    p_app->handleInput();

    // Swap front/back framebuffers
    SDL_GL_SwapWindow(p_app->m_window.getHandle());
    auto toc = clock();
    // auto toc =  std::chrono::high_resolution_clock::now();

    // std::cout << "frame took: " << std::chrono::duration_cast<std::chrono::microseconds>(toc - tic) << "\n";
    double dt = (double)(toc - tic) / CLOCKS_PER_SEC * 1000.f;
    p_app->m_avg_frame_time.addNumber(dt);
    if (dt < 16.6666)
    {
        SDL_Delay(16.6666 - dt);
    }

#ifdef __EMSCRIPTEN__
    // emscripten_trace_record_frame_end();
#endif
}

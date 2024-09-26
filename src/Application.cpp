#include "Application.h"
#include "PostEffects.h"
#include "DrawLayer.h"
#include "LuaWrapper.h"

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

void drawProgramToTexture(Sprite2 &rect, Renderer &target, std::string program)
{
    target.clear({1, 1, 1, 1});
    target.drawSprite(rect, program, GL_DYNAMIC_DRAW);
    target.drawAll();
}

struct EdgeLord
{

public:
};

void updateTriangulation(cdt::Triangulation<cdt::Vector2i> &cdt, MapGridDiagonal &map, SurfaceManager &surfaces)
{
    cdt.reset();
    map.extractBoundaries();
    auto edges = map.extractEdges();
    std::vector<cdt::EdgeVInd> edge_inds;
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
    if (!surfaces.readFromMap(map))
    {
        throw std::runtime_error("Map is fucked :(");
    }
    GameWorld::getWorld().getPathFinder().update();
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
    text_layer.m_canvas.addShader("Text", "../Resources/basicinstanced.vert", "../Resources/textBorder.frag");

    auto &unit_layer = m_layers.addLayer("Unit", 3, options);
    // unit_layer.addEffect(std::make_unique<Bloom2>(width, height));
    unit_layer.m_canvas.addShader("Shiny", "../Resources/basicinstanced.vert", "../Resources/shiny.frag");
    unit_layer.m_canvas.addShader("lightning", "../Resources/basicinstanced.vert", "../Resources/lightning.frag");

    auto &smoke_layer = m_layers.addLayer("Smoke", 4, options);
    // smoke_layer.addEffect(std::make_unique<BloomSmoke>(width, height));

    auto &fire_layer = m_layers.addLayer("Fire", 6, options);
    fire_layer.addEffect(std::make_unique<Bloom>(width, height));

    auto &wall_layer = m_layers.addLayer("Wall", 0, options);
    wall_layer.addEffect(std::make_unique<Bloom2>(width, height, options));

    auto &light_layer = m_layers.addLayer("Light", 150, options);
    light_layer.m_canvas.m_blend_factors = {BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha};
    light_layer.addEffect(std::make_unique<SmoothLight>(width, height));
    light_layer.addEffect(std::make_unique<LightCombine>(width, height));
    light_layer.m_canvas.addShader("VisionLight", "../Resources/basictex.vert", "../Resources/fullpassLight.frag");
    light_layer.m_canvas.addShader("combineBloomBetter", "../Resources/basicinstanced.vert", "../Resources/combineBloomBetter.frag");

    auto &water_layer = m_layers.addLayer("Water", 3, options);
    // water_layer.addEffect(std::make_unique<WaterEffect>(width, height));
    //
}

Application::Application(int width, int height) : m_window(width, height),
                                                  m_window_renderer(m_window),
                                                  m_scene_pixels(width, height),
                                                  m_scene_canvas(m_scene_pixels)
{
    using namespace map;

    initializeLayers();

    m_font = std::make_shared<Font>("arial.ttf");
    
    m_map = std::make_unique<MapGridDiagonal>(utils::Vector2i{MAP_SIZE_X, MAP_SIZE_Y}, utils::Vector2i{MAP_GRID_CELLS_X, MAP_GRID_CELLS_Y});
    auto &world = GameWorld::getWorld();
    for (int i = 0; i < 50; ++i)
    {
        auto rand_pos = randomPosInBox({5, 5}, {1900, 1900});
        auto map_cell = m_map->coordToCell(rand_pos);
        int rx = 1 + rand() % 4;
        int ry = 1 + rand() % 4;
        m_map->changeTiles(MapGridDiagonal::Tile::Wall, {m_map->cellCoordX(map_cell), m_map->cellCoordY(map_cell)}, {rx, ry});
    }

    updateTriangulation(world.getTriangulation(), *m_map, m_surfaces);

    p_player = world.addObject(ObjectType::Player, "Player");
    p_player->setPosition({500, 500});
    // world.addObject(ObjectType::Orbiter, "Shield", world.getIdOf("Player"));

    world.update(0); //! force insert player to world

    for (int i = 0; i < 0; ++i)
    {
        auto new_enemy = world.addObject("Enemy", "E" + std::to_string(i), -1);
        if (new_enemy)
        {
            auto rand_pos = randomPosInBox({5, 5}, {1900, 1900});
            new_enemy->setPosition(rand_pos);
            static_cast<Enemy &>(*new_enemy).m_target = p_player.get();
            static_cast<Enemy &>(*new_enemy).m_target_pos = p_player->getPosition();
        }
    }

    std::filesystem::path path{"../Resources/"};
    auto shader_filenames = extractNamesInDirectory(path, ".frag");

    // assert(m_layers.hasLayer("Water"));
    for (auto &filename : shader_filenames)
    {
        auto pos_right = filename.find_last_of('.');
        std::string shader_name = filename.substr(0, pos_right);
        // m_window_renderer.addShader(shader_name, "../Resources/basicinstanced.vert",  "../Resources/" + filename);
    }
    m_window_renderer.addShader("Shiny", "../Resources/basicinstanced.vert", "../Resources/shiny.frag");
    m_window_renderer.addShader("Water", "../Resources/basictex.vert", "../Resources/test.frag");
    m_window_renderer.addShader("Instanced", "../Resources/basicinstanced.vert", "../Resources/texture.frag");
    m_window_renderer.addShader("LastPass", "../Resources/basicinstanced.vert", "../Resources/lastPass.frag");
    m_window_renderer.addShader("VertexArrayDefault", "../Resources/basictex.vert", "../Resources/fullpass.frag");
    m_window_renderer.addShader("Text", "../Resources/basicinstanced.vert", "../Resources/textBorder.frag");
    m_scene_canvas.addShader("Instanced", "../Resources/basicinstanced.vert", "../Resources/texture.frag");
    m_scene_canvas.addShader("gaussHoriz", "../Resources/basicinstanced.vert", "../Resources/gaussHoriz.frag");
    m_scene_canvas.addShader("VertexArrayDefault", "../Resources/basictex.vert", "../Resources/fullpass.frag");
    m_scene_canvas.addShader("combineBloom", "../Resources/basicinstanced.vert", "../Resources/combineBloom.frag");
    m_scene_canvas.addShader("combineBloomBetter", "../Resources/basicinstanced.vert", "../Resources/combineBloomBetter.frag");
    m_scene_canvas.addShader("combineSmoke", "../Resources/basicinstanced.vert", "../Resources/combineSmoke.frag");
    m_scene_canvas.addShader("combineEdges", "../Resources/basicinstanced.vert", "../Resources/combineEdges.frag");

    // m_water = std::make_shared<Water>(m_window_renderer.getShaders(), m_layers);
    auto &water = world.addVisualEffect<Water>("Water");
    // auto effect = world.addEffect("Fire", "TestFire", -1);
    // effect->setPosition(p_player->getPosition());

    auto texture_filenames = extractNamesInDirectory(path, ".png");
    for (auto &texture_filename : texture_filenames)
    {
        auto pos_right = texture_filename.find_last_of('.');
        std::string texture_name = texture_filename.substr(0, pos_right);
        m_textures.add(texture_name, "../Resources/" + texture_filename);
    }

    //! set view and add it to renderers
    m_window_renderer.m_view.setSize(m_window.getSize());
    m_window_renderer.m_view.setCenter(m_window.getSize() / 2);

    // m_layers.activate("Light");
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
        m_is_moving_left = true;
        break;
    case SDLK_d:
        m_is_moving_right = true;
        break;
    case SDLK_w:
        m_is_moving_up = true;
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
        p_player->setTarget(static_cast<Enemy *>(selected_enemies[0]));
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
        selectInWorld(m_selection_click_pos, m_window_renderer.getMouseInWorld());
        m_is_selecting = false;
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
        }
    }
}

void Application::onKeyRelease(SDL_Keycode key)
{
    auto mouse_pos = m_window_renderer.getMouseInWorld();

    switch (key)
    {
    case SDLK_a:
        m_is_moving_left = false;
        break;
    case SDLK_d:
        m_is_moving_right = false;
        break;
    case SDLK_w:
        m_is_moving_up = false;
        break;
    case SDLK_s:
        m_is_moving_down = false;
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
    if (p_player->m_target)
    {
        if (p_player->m_target->isDead())
        {
            p_player->m_target = nullptr;
            return;
        }
        auto &projectile = static_cast<Projectile &>(*world.addObject(ObjectType::Bullet, "Projectile"));
        projectile.setScriptName("frostbolt");
        projectile.setPosition(from);
        projectile.setSize(10.f);
        projectile.setTarget(p_player->m_target);
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
        p_player->m_vel.x = 30.f * ((int)m_is_moving_right - (int)m_is_moving_left);
        p_player->m_vel.y = 30.f * ((int)m_is_moving_up - (int)m_is_moving_down);
        int a = m_is_moving_down + m_is_moving_left + m_is_moving_up + m_is_moving_right;
        if (a >= 2)
        {
            p_player->m_vel /= std::sqrt(2.f);
        }
        if (m_is_sprinting)
        {
            p_player->m_vel *= 3.f;
        }
    }
    world.update(0.1f);
    m_time += 0.1f;
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
    Sprite2 background(*m_textures.get("grass"));
    background.m_color = ColorByte{255, 255, 255, 0};
    utils::Vector2f map_size = {m_map->m_cell_count.x * m_map->m_cell_size.x,
                                m_map->m_cell_count.y * m_map->m_cell_size.y};
    background.setPosition(map_size / 2.f);
    background.setScale(map_size / 2.f);
    auto old_view = m_scene_canvas.m_view;
    m_scene_canvas.m_view = m_window_renderer.m_view;
    m_scene_canvas.drawSprite(background, "Instanced", GL_DYNAMIC_DRAW);
    m_scene_canvas.drawAll();
    m_scene_canvas.m_view = old_view;
    m_layers.draw(m_scene_canvas, m_window_renderer.m_view);

    // //! draw everything to a window quad
    m_window.clear({0, 0, 0, 0});
    auto scene_size = m_scene_pixels.getSize();

    old_view = m_window_renderer.m_view;
    Sprite2 screen_sprite(m_scene_pixels.getTexture());
    screen_sprite.setPosition(scene_size / 2.f);
    screen_sprite.setScale(scene_size / 2.f);
    m_window_renderer.m_view.setCenter(screen_sprite.getPosition());
    m_window_renderer.m_view.setSize(scene_size);
    m_window_renderer.drawSprite(screen_sprite, "LastPass", GL_DYNAMIC_DRAW);
    auto old_factors = m_window_renderer.m_blend_factors;
    m_window_renderer.m_blend_factors = {BlendFactor::One, BlendFactor::OneMinusSrcAlpha};
    m_window_renderer.drawAll();
    m_window_renderer.m_blend_factors = old_factors;
    m_window_renderer.m_view = old_view;

    Text text;
    m_window_renderer.m_view.setCenter(m_window.getSize() / 2);
    m_window_renderer.m_view.setSize(m_window.getSize());
    text.setPosition(100, 0);
    text.setScale(1, 1);
    text.setFont(m_font);
    text.setColor({255, 255, 255, 255});
    std::string text_test = "Frame time: ... " + std::to_string(m_avg_frame_time.avg);
    text.setText(text_test);
    m_window_renderer.drawText(text, "Text", GL_DYNAMIC_DRAW);

    Sprite2 health_bar(*m_textures.get("bomb"));
    health_bar.m_texture_handles[0] = 0;

    float x_scale = p_player->m_health / (double)p_player->m_max_health;
    utils::Vector2f window_size = m_window.getSize();
    health_bar.setPosition(0.2 * window_size.x, 0.95 * window_size.y);
    health_bar.setScale(100, 15);
    m_window_renderer.drawSprite(health_bar, "healthBar", GL_DYNAMIC_DRAW);
    m_window_renderer.getShader("healthBar").getVariables().uniforms["u_health_percentage"] = x_scale;
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

    p_app->update(0);
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

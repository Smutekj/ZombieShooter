#include "Application.h"
#include "PostEffects.h"
#include "DrawLayer.h"

#include <IncludesGl.h>
#include <Utils/RandomTools.h>
#include <Utils/IO.h>

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

Application::Application(int width, int height) : m_window(width, height),
                                                  m_window_renderer(m_window),
                                                  m_scene_pixels(width, height),
                                                  m_scene_canvas(m_scene_pixels),
                                                  m_cdt({800, 600}),
                                                  m_vision(m_cdt)

{

    m_map = std::make_unique<MapGridDiagonal>(utils::Vector2i{800, 600}, utils::Vector2i{80, 60});

    auto& world = GameWorld::getWorld();
    p_player = world.addObject(ObjectType::Player);
    p_player->setPosition({400, 300});
    p_player->setSize({50, 50});

    std::filesystem::path path{"../Resources/"};
    auto shader_filenames = extractNamesInDirectory(path, ".frag");

    auto &unit_layer = m_layers.addLayer("Unit", 5);
    unit_layer.addEffect(std::make_unique<Bloom2>(width, height));
    unit_layer.m_canvas.addShader("Shiny", "../Resources/basicinstanced.vert", "../Resources/shiny.frag");
    auto &smoke_layer = m_layers.addLayer("Smoke", 4);
    smoke_layer .addEffect(std::make_unique<BloomSmoke>(width, height));
    auto &fire_layer = m_layers.addLayer("Fire", 2);
    fire_layer.addEffect(std::make_unique<Bloom2>(width, height));
    auto &wall_layer = m_layers.addLayer("Wall", 0);
    wall_layer.addEffect(std::make_unique<Bloom2>(width, height));
    // auto &edge_detect = m_layers.addLayer("Light", 0);
    // edge_detect.addEffect(std::make_unique<LightCombine>(width, height));
    auto &water_layer = m_layers.addLayer("Water", 3);
    // water_layer.addEffect(std::make_unique<WaterEffect>(width, height));
    //
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
    m_scene_canvas.addShader("Instanced", "../Resources/basicinstanced.vert", "../Resources/texture.frag");
    m_scene_canvas.addShader("gaussHoriz", "../Resources/basicinstanced.vert", "../Resources/gaussHoriz.frag");
    m_scene_canvas.addShader("VertexArrayDefault", "../Resources/basictex.vert", "../Resources/fullpass.frag");
    m_scene_canvas.addShader("combineBloom", "../Resources/basicinstanced.vert", "../Resources/combineBloom.frag");
    m_scene_canvas.addShader("combineBloomBetter", "../Resources/basicinstanced.vert", "../Resources/combineBloomBetter.frag");
    m_scene_canvas.addShader("combineSmoke", "../Resources/basicinstanced.vert", "../Resources/combineSmoke.frag");
    m_scene_canvas.addShader("combineEdges", "../Resources/basicinstanced.vert", "../Resources/combineEdges.frag");

    m_water = std::make_shared<Water>(m_window_renderer.getShaders(), m_layers);
    m_enviroment.push_back(m_water);
    // m_enviroment.push_back(std::make_unique<FireEffect>());
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
    case SDLK_LEFT:
        m_is_moving_left = true;
        break;
    case SDLK_RIGHT:
        m_is_moving_right = true;
        break;
    case SDLK_UP:
        m_is_moving_up = true;
        break;
    case SDLK_DOWN:
        m_is_moving_down = true;
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
}
std::vector<int> findConnectedTriInds(cdt::Triangulation<cdt::Vector2i> &cdt, int start_tri_ind)
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

void Application::onMouseButtonRelease(SDL_MouseButtonEvent event)
{
    auto mouse_pos = m_window_renderer.getMouseInWorld();
    if (event.button == SDL_BUTTON_MIDDLE)
    {
        m_wheel_is_held = false;
    }
    if (isKeyPressed(SDL_SCANCODE_LCTRL) && event.button == SDL_BUTTON_RIGHT)
    {
        std::cout << "mouse: " << m_window_renderer.m_view.getScale().x << " " << m_window_renderer.m_view.getScale().y << "\n";
        auto map_cell = m_map->coordToCell(mouse_pos.x, mouse_pos.y);
        std::cout << m_map->cellCoordX(map_cell) << " " << m_map->cellCoordY(map_cell) << "\n";
        m_map->changeTiles(MapGridDiagonal::Tile::Wall, {m_map->cellCoordX(map_cell), m_map->cellCoordY(map_cell)}, {5, 5});
        m_cdt.reset();
        m_map->extractBoundaries();
        auto edges = m_map->extractEdges();
        std::vector<cdt::EdgeVInd> edge_inds;
        for (auto &e : edges)
        {
            cdt::EdgeVInd e_ind;
            auto v_ind1 = m_cdt.insertVertexAndGetData(e.from).overlapping_vertex;
            e_ind.from = (v_ind1 == -1 ? m_cdt.m_vertices.size() - 1 : v_ind1);

            auto v_ind2 = m_cdt.insertVertexAndGetData(e.to()).overlapping_vertex;
            e_ind.to = (v_ind2 == -1 ? m_cdt.m_vertices.size() - 1 : v_ind2);

            edge_inds.push_back(e_ind);
        }
        for (auto &e : edge_inds)
        {
            m_cdt.insertConstraint(e);
        }
    }
    if (isKeyPressed(SDL_SCANCODE_LSHIFT) && event.button == SDL_BUTTON_RIGHT)
    {
        auto tri_ind = m_cdt.findTriangle({mouse_pos.x, mouse_pos.y});
        auto connected_inds = findConnectedTriInds(m_cdt, tri_ind);
        m_water->readFromMap(m_cdt, connected_inds);
    }
}

void Application::onKeyRelease(SDL_Keycode key)
{
    auto mouse_pos = m_window_renderer.getMouseInWorld();

    switch (key)
    {
    case SDLK_LEFT:
        m_is_moving_left = false;
        break;
    case SDLK_RIGHT:
        m_is_moving_right = false;
        break;
    case SDLK_UP:
        m_is_moving_up = false;
        break;
    case SDLK_DOWN:
        m_is_moving_down = false;
        break;
    case SDLK_ESCAPE:
        m_window.close();
        break;
    }
}

void Application::fireProjectile(ProjectileTarget target, utils::Vector2f from)
{
    auto& world = GameWorld::getWorld();
    auto &projectile = static_cast<Projectile &>(*world.addObject(ObjectType::Bullet));
    projectile.setPosition(from);
    projectile.setSize(10.f);
    projectile.setTarget(target);
}

void moveView(utils::Vector2f dr, Renderer &target)
{
    auto &view = target.m_view;
    auto old_view_center = view.getCenter();
    auto new_view_center = old_view_center - (dr);
    view.setCenter(new_view_center.x, new_view_center.y);
}

void drawAgent(utils::Vector2f pos, float radius, LayersHolder &layers, Color color)
{
    auto &canvas = layers.getCanvas("Unit");

    utils::Vector2f prev_pos = pos + utils::Vector2f{radius, 0};
    for (int i = 1; i < 50; ++i)
    {
        float x = pos.x + radius * std::cos(i / 50. * 2. * M_PIf);
        float y = pos.y + radius * std::sin(i / 50. * 2. * M_PIf);
        canvas.drawLineBatched(prev_pos, {x, y}, 1.0, color, GL_DYNAMIC_DRAW);
        prev_pos = {x, y};
    }
    utils::Vector2f left_eye_pos1 = pos + utils::Vector2f{-radius * 0.2, radius * 0.4};
    utils::Vector2f left_eye_pos2 = pos + utils::Vector2f{-radius * 0.6, radius * 0.5};
    utils::Vector2f right_eye_pos1 = pos + utils::Vector2f{+radius * 0.2, radius * 0.4};
    utils::Vector2f right_eye_pos2 = pos + utils::Vector2f{+radius * 0.6, radius * 0.5};
    canvas.drawLineBatched(left_eye_pos1, left_eye_pos2, 1.0, color, GL_DYNAMIC_DRAW);
    canvas.drawLineBatched(right_eye_pos1, right_eye_pos2, 1.0, color, GL_DYNAMIC_DRAW);
    // canvas.drawCricleBatched(, 1.0, color, GL_DYNAMIC_DRAW);
}

Particles m_bolt_particles(500);

float inline dir2angle(const utils::Vector2f &dir)
{
    return 180.f / M_PIf * std::atan2(dir.y, dir.x);
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

    auto& world = GameWorld::getWorld();
    world.update(0.016f);
    m_time += 0.016f;

    auto &wall_canvas = m_layers.getCanvas("Wall");
    wall_canvas.m_view = m_window_renderer.m_view;

    auto &unit_canvas = m_layers.getCanvas("Unit");
    unit_canvas.m_view = m_window_renderer.m_view;

    auto mouse_coords = m_window_renderer.getMouseInWorld();
    m_enviroment.back()->setPosition(mouse_coords);
    m_layers.clearAllLayers();
    for (auto &effect : m_enviroment)
    {
        effect->draw(m_layers, m_window_renderer.m_view);
    }
    m_window.clear({0, 0, 0, 1});
    drawTriangles(m_cdt, wall_canvas, m_ui->getBackgroundColor());
    world.draw(m_layers);
    // auto& light_canvas = m_layers.getCanvas("Light");
    // light_canvas.m_view = m_window_renderer.m_view;
    // light_canvas.drawCricleBatched({mouse_coords.x, mouse_coords.y}, 45.f/180.f*3.145f, 80, 100, {1,1,1,0.5});

    // // // clear and draw into scene
    m_scene_canvas.clear({0, 0, 0, 1});
    auto &vars = m_window_renderer.getShader("Water").getVariables();
    vars.uniforms.at("u_time") = m_time;
    auto &vars2 = m_window_renderer.getShader("Shiny").getVariables();
    vars2.uniforms.at("u_time") = m_time;

    // m_window_renderer.drawCricleBatched({10.*m_time, 400.f}, 20, {1, 1,0,1});
    // m_window_renderer.drawCricleBatched({20.*m_time, 400.f}, 20, {1, 0,0,1});
    // drawAgent(mouse_coords, 10, m_layers, m_ui->getParticleEndColor());
    // // // Sprite2 background(*m_textures.get("TestImage"));
    // // // background.setPosition(m_scene_canvas.getTargetSize()/2.f);
    // // // background.setScale(m_scene_canvas.getTargetSize()/2.f);
    // // // m_scene_canvas.drawSprite(background, "Instanced", GL_DYNAMIC_DRAW);
    // m_scene_canvas.drawAll();
    Sprite2 test_sprite(*m_textures.get("coin"));
    test_sprite.setScale(utils::Vector2f{200., 150.});
    test_sprite.setPosition(utils::Vector2f{400., 300.});
    // unit_canvas.drawSprite(test_sprite, "Shiny", GL_DYNAMIC_DRAW);
    m_layers.draw(m_scene_canvas);
    //! draw everything to a window quad
    // m_window.clear({0, 0, 0, 0});
    auto old_view = m_window_renderer.m_view;
    Sprite2 screen_sprite(m_scene_pixels.getTexture());
    auto scene_size = m_scene_pixels.getSize();
    screen_sprite.setPosition(scene_size / 2.f);
    screen_sprite.setScale(scene_size / 2.f);
    m_window_renderer.m_view.setCenter(screen_sprite.getPosition());
    m_window_renderer.m_view.setSize(scene_size);
    m_window_renderer.drawSprite(screen_sprite, "LastPass", GL_DYNAMIC_DRAW);
    m_window_renderer.drawAll();

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
    std::cout << "frame took: " << (dt) << "\n";
    // SDL_Delay(10);
#ifdef __EMSCRIPTEN__
    // emscripten_trace_record_frame_end();
#endif
}

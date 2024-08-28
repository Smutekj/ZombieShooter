// #include <omp.h>
// #include <iostream>
// #include <fstream>
// #include <unordered_set>

// #include "Game.h"

// #include <iostream>

// Game::Game(sf::RenderWindow &window, KeyBindings &bindings)
//     : m_window(window), key_binding(bindings), m_searcher()
// {

//     ImGui::SFML::Init(m_window);

//     utils::Vector2f box_size = {500, 500};
//     utils::Vector2f cell_size = {4, 4};
//     utils::Vector2i n_cells = {(int)(box_size.x / cell_size.x), (int)(box_size.y / cell_size.y)};

//     m_grid = std::make_shared<SearchGrid>(n_cells, cell_size);
//     m_cdt = std::make_shared<Triangulation>(*m_grid);
//     m_cdt->createBoundaryAndSuperTriangle();
//     m_player_vision = std::make_unique<VisionField>(*m_cdt);
//     m_pathfinder = std::make_unique<PathFinder>(m_cdt.get());
//     m_pathfinder->update();

//     m_map = std::make_unique<MapGrid>(n_cells, box_size, cell_size);
//     m_player.pos = box_size / 2.f;

//     auto view = window.getView();
//     view.setCenter({box_size / 2.f});
//     view.setSize({100.f, 100.f * window.getSize().y / window.getSize().x});
//     window.setView(view);

//     m_light_texture.create(1000, 600);
//     m_light_texture.setSmooth(true);
//     m_light_cut.create(800, 600);
//     m_light_cut.setSmooth(true);
//     m_texture_pass[0].create(800, 600);
//     m_texture_pass[1].create(800, 600);
//     m_texture_pass[0].setSmooth(true);
//     m_texture_pass[1].setSmooth(true);
//     m_light_smoother.loadFromFile("../Resources/basic.vert", "../Resources/lightSmootherRGB.frag");
//     m_full_pass.loadFromFile("../Resources/basic.vert", "../Resources/fullpass.frag");

//     m_font.loadFromFile("../Resources/arial.ttf");

//     for (int i = 0; i < 100; ++i)
//     {
//         utils::Vector2f rand_pos = box_size / 2.f + 50.f * angle2dir(randf(0, 360));
//         Zombie z;
//         z.pos = rand_pos;
//         z.target = box_size / 2.f; //;rayCast(z.pos, angle2dir(randf(0, 0)), m_map->box_size_.x / 2.f, *m_cdt);
//         z.radius = 1.f;
//         m_zombies.addObject(z);
//         m_searcher.insert(z.pos, i);
//     }
// }

// void Game::moveView(sf::RenderWindow &window)
// {
//     // const utils::Vector2f view_size = view.getSize();

//     auto mouse_window_pos = sf::Mouse::getPosition(m_window);
//     auto view = m_window.getView();
//     auto window_size = m_window.getSize();
//     if (mouse_window_pos.x > window_size.x - 5)
//     {
//         view.move({2.5f, 0});
//     }
//     if (mouse_window_pos.x < 10)
//     {
//         view.move({-2.5f, 0});
//     }
//     if (mouse_window_pos.y > window_size.y - 5)
//     {
//         view.move({0, 2.5f});
//     }
//     if (mouse_window_pos.y < 10)
//     {
//         view.move({0, -2.5f});
//     }

//     auto threshold = view.getSize() / 2.f - view.getSize() / 3.f;
//     auto dx = m_player.pos.x - view.getCenter().x;
//     auto dy = m_player.pos.y - view.getCenter().y;
//     auto view_max = view.getCenter() + view.getSize() / 2.f;
//     auto view_min = view.getCenter() - view.getSize() / 2.f;

//     //! move view when approaching sides
//     if (dx > threshold.x && view_max.x < Geometry::BOX[0])
//     {
//         view.setCenter(view.getCenter() + utils::Vector2f{dx - threshold.x, 0});
//     }
//     else if (dx < -threshold.x && view_min.x > 0)
//     {
//         view.setCenter(view.getCenter() + utils::Vector2f{dx + threshold.x, 0});
//     }
//     if (dy > threshold.y && view_max.y < Geometry::BOX[1])
//     {
//         view.setCenter(view.getCenter() + utils::Vector2f{0, dy - threshold.y});
//     }
//     else if (dy < -threshold.y && view_min.y > 0)
//     {
//         view.setCenter(view.getCenter() + utils::Vector2f{0, dy + threshold.y});
//     }

//     window.setView(view);
// }

// void combineTextures(const sf::RenderTexture &source, sf::RenderTarget &destination,
//                      sf::Shader &shader, sf::BlendMode blend_mode)
// {
//     sf::VertexArray texture_rect;
//     texture_rect.resize(4);
//     texture_rect.setPrimitiveType(sf::Quads);
//     utils::Vector2f rect_size(destination.getSize().x, destination.getSize().y);
//     texture_rect[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 1}};
//     texture_rect[1] = sf::Vertex{{rect_size.x, 0}, sf::Color::Transparent, {1, 1}};
//     texture_rect[2] = sf::Vertex{{rect_size.x, rect_size.y}, sf::Color::Transparent, {1, 0}};
//     texture_rect[3] = sf::Vertex{{0, rect_size.y}, sf::Color::Transparent, {0, 0}};

//     shader.setUniform("image", source.getTexture());
//     sf::RenderStates states;
//     states.blendMode = blend_mode;
//     states.shader = &shader;
//     destination.setView(destination.getDefaultView());
//     destination.draw(texture_rect, states);
// }

// bool segmentIntersectsCircle(const Edgef &segment, const utils::Vector2f r, const float radius, utils::Vector2f &intersection)
// {
//     const auto dr = segment.from - r;
//     utils::Vector2f n = {-segment.t.y, segment.t.x};
//     if (dist(segment.from, r) < radius || dist(segment.to(), r) < radius)
//     {
//         return true;
//     }
//     const auto norm_dr_sq = dot(dr, dr);
//     const auto dot_dr_t = dot(dr, segment.t);
//     const auto discriminant = dot_dr_t * dot_dr_t - norm_dr_sq + radius * radius;
//     if (discriminant >= 0)
//     {
//         const auto sqrt_disc = std::sqrt(discriminant);
//         auto a1 = -dot_dr_t + sqrt_disc;
//         auto a2 = -dot_dr_t - sqrt_disc;
//         bool point1_is_on_edge = (a1 <= segment.l and a1 > 0);
//         bool point2_is_on_edge = (a2 <= segment.l and a2 > 0);
//         if (point1_is_on_edge)
//         {
//             intersection = segment.from + a1 * segment.t;
//         }
//         if (point2_is_on_edge)
//         {
//             intersection = segment.from + a2 * segment.t;
//         }
//         return (point2_is_on_edge) || (point1_is_on_edge);
//     }
//     return false;
// }

// void Game::shootGun(utils::Vector2f from, utils::Vector2f dir)
// {
//     m_shots.clear();
//     float length = 50.f;
//     float spread_angle = 30;
//     for (int i = -10; i <= 10; ++i)
//     {
//         float angle = dir2angle(dir) + i * spread_angle / 20.f;
//         auto bullet_dir = angle2dir(angle);
//         utils::Vector2f r_end = rayCast(from, bullet_dir, length, *m_cdt);
//         utils::Vector2f hit_point;
//         float min_dist2 = std::numeric_limits<float>::max();
//         Zombie *z = nullptr;
//         int k = 0;
//         int k_hit = -1;
//         for (auto &zombie : m_zombies.getObjects())
//         {
//             utils::Vector2f new_hit;
//             if (segmentIntersectsCircle({from, r_end}, zombie.pos, zombie.radius, new_hit))
//             {
//                 auto new_dist2 = dist2(hit_point, from);
//                 if (new_dist2 < min_dist2)
//                 {
//                     zombie.impulse_vel += bullet_dir * m_hit_force;

//                     k_hit = k;
//                     z = &zombie;
//                     min_dist2 = new_dist2;
//                     hit_point = new_hit;
//                 }
//             }
//             k++;
//         }
//         if (z)
//         {
//             // z->impulse_vel += bullet_dir*50.f;
//         }

//         m_shots.push_back({from, r_end});
//     }
// }

// void Game::handleEvent(const sf::Event &event)
// {
//     auto mouse_position = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));

//     ImGui::SFML::ProcessEvent(event);

//     if (m_changing_map)
//     {
//         if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Button::Right)
//         {
//             auto clicked_tri_ind = m_cdt->findTriangle(mouse_position);
//             m_selected_tri_1 == -1 ? m_selected_tri_1 = clicked_tri_ind : m_selected_tri_2 = clicked_tri_ind;
//         }

//         if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Key::Escape)
//         {
//             m_selected_tri_1 = -1;
//             m_selected_tri_2 = -1;
//         }
//     }
//     else
//     {


//         if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Button::Right)
//         {
//             utils::Vector2f dir = mouse_position - m_player.pos;
//             shootGun(m_player.pos, dir / norm(dir));
//         }
//     }

//     if (event.type == sf::Event::MouseWheelMoved)
//     {
//         auto view = m_window.getView();
//         if (event.mouseWheelScroll.wheel > 0)
//         {
//             view.zoom(0.9f);
//         }
//         else
//         {
//             view.zoom(1. / 0.9f);
//         }
//         m_window.setView(view);
//     }
// }

// //! \brief parse events and normal input
// //! \note  right now this is just a placeholder code until I make a nice OOP solution with bindings and stuff
// void Game::parseInput(sf::RenderWindow &window)
// {
//     auto mouse_position = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));

//     if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
//     {
//         const utils::Vector2i building_size = {2, 2};
//         auto lower_left_cell_coord = m_map->drawProposedBuilding(m_window, mouse_position, building_size);
//         m_map->buildWall(mouse_position, building_size);
//         m_map->updateBoundaryTypesLocally(lower_left_cell_coord, building_size);
//         m_map->sawOffCorners();
//         m_map->extractEdgesFromTilesV2(*m_cdt);
//         m_map->updateTexture();
//         m_pathfinder->update();
//     }

//     bool w_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
//     bool a_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
//     bool s_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
//     bool d_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
//     if (w_pressed)
//     {
//         // m_player.pos += 2.f * angle2dir(m_player.angle);
//         m_player.vel.y = -1.5;
//     }
//     if (a_pressed)
//     {
//         m_player.vel.x = -1.5f;
//         // m_player.angle -= 5.f;
//     }
//     if (s_pressed)
//     {
//         // m_player.pos.x+= 2.f;
//         m_player.vel.y = 1.5f;
//     }
//     if (d_pressed)
//     {
//         m_player.vel.x = +1.5f;
//         // m_player.angle += 5.f;
//     }
//     if (!(w_pressed || a_pressed || s_pressed || d_pressed))
//     {
//         m_player.vel *= 0.f;
//     }
// }

// void drawLine(sf::RenderWindow &window, utils::Vector2f p1, utils::Vector2f p2, sf::Color color = sf::Color::Green)
// {
//     sf::RectangleShape line;
//     line.setFillColor(color);
//     line.setPosition(p1);
//     utils::Vector2f dr = p2 - p1;
//     line.setSize({norm(dr), 0.5});
//     line.setRotation(dir2angle(dr));

//     window.draw(line);
// }

// void Game::drawTriInd(sf::RenderWindow &window, Triangle &tri, TriInd ind)
// {
//     sf::Text num;
//     num.setFont(m_font);
//     num.setString(std::to_string(ind));
//     utils::Vector2f center = utils::Vector2f(tri.verts[0] + tri.verts[1] + tri.verts[2]) / 3.f;
//     num.setPosition(center); // - utils::Vector2f(num.getGlobalBounds().width, num.getLocalBounds().height)
//     ind == m_selected_tri_1 || ind == m_selected_tri_2 ? num.setFillColor(sf::Color::Red) : num.setFillColor(sf::Color::Green);
//     num.setScale(0.1f, 0.1f);
//     window.draw(num);
// }

// void drawTriangle(sf::RenderWindow &window, Triangle &tri, sf::Color color)
// {
//     sf::VertexArray t;
//     t.setPrimitiveType(sf::Triangles);
//     t.resize(3);
//     t[0] = {asFloat(tri.verts[0]), color};
//     t[1] = {asFloat(tri.verts[1]), color};
//     t[2] = {asFloat(tri.verts[2]), color};
//     window.draw(t);
// }

// void Game::applyWallConstraint()
// {
//     auto &zombies = m_zombies.getObjects();
//     const auto n_comps = zombies.size();

//     for (int comp_ind = 0; comp_ind < n_comps; ++comp_ind)
//     {
//         auto &comp = zombies.at(comp_ind);
//         const auto &r_selected = comp.pos;
//         auto &vel = comp.vel;
//         auto &v_inertial = comp.impulse_vel;
//         const auto radius = comp.radius;
//         const auto walls = m_map->p_edges_->calcContactEdgesO(r_selected, radius);

//         bool is_touching_wall = false;
//         for (int orient = 0; orient < 4; orient++)
//         {
//             for (const auto &wall : walls[orient])
//             {
//                 is_touching_wall = true;
//                 utils::Vector2f n_wall = {wall.t.y, -wall.t.x};

//                 auto v_away_from_surface = n_wall * dot(vel, n_wall);
//                 if (dot(vel, n_wall) < 0)
//                 {
//                     vel -= 1.0f * v_away_from_surface;
//                 }
//                 v_away_from_surface = n_wall * dot(comp.impulse_vel, n_wall);
//                 if (dot(comp.impulse_vel, n_wall) < 0)
//                 {
//                     comp.impulse_vel -= v_away_from_surface;
//                 }
//                 v_away_from_surface = n_wall * dot(comp.impulse, n_wall);
//                 if (dot(comp.impulse, n_wall) < 0)
//                 {
//                     comp.impulse -= v_away_from_surface;
//                 }
//             }
//         }
//     }
//     const auto walls = m_map->p_edges_->calcContactEdgesO(m_player.pos, m_player.radius);
//     auto &vel = m_player.vel; // angle2dir(m_player.angle)*m_player.speed;
//     for (int orient = 0; orient < 4; orient++)
//     {
//         for (const auto &wall : walls[orient])
//         {
//             utils::Vector2f n_wall = {wall.t.y, -wall.t.x};

//             auto v_away_from_surface = n_wall * dot(vel, n_wall);
//             if (dot(vel, n_wall) < 0)
//             {
//                 vel -= 1.0f * v_away_from_surface;
//             }
//         }
//     }
//     // m_player.speed = norm(vel);
//     // m_player.angle = dir2angle(vel);
// }

// void Game::update(const float dt, sf::RenderWindow &window)
// {
//     moveView(window);
//     parseInput(window);

//     if (!m_changing_map)
//     {
//         auto isInMap = [this](utils::Vector2f pos)
//         {
//             auto box = m_map->box_size_;
//             return pos.x <= box.x && pos.y <= box.y && pos.x >= 0 && pos.y >= 0;
//         };

//         int i = 0;
//         for (auto &zombie : m_zombies.getObjects())
//         {
//             auto path = m_pathfinder->doPathFinding(zombie.pos, m_player.pos, zombie.radius);
//             zombie.target = path.path[1];
//             auto neighbours = m_searcher.getNeighboursOf(zombie.pos, 20 * zombie.radius);

//             for (auto neighbour : neighbours)
//             {
//                 if (neighbour == i)
//                 {
//                     continue;
//                 }

//                 auto &neig = m_zombies.at(neighbour);
//                 auto n_vel = neig.vel;

//                 auto dr = neig.pos - zombie.pos;
//                 auto r_collision_sq = (zombie.radius + neig.radius) * (zombie.radius + neig.radius);
//                 const auto x = r_collision_sq / (norm2(dr));
//                 if (x > 0.8f)
//                 {
//                     zombie.impulse += m_repulse_multiplier * (-dr) * x * x;
//                 }
//             }

//             float path_width = zombie.radius * 2.f;
//             auto dr_pathpoint = zombie.path_point - zombie.pos;
//             auto t_pp = dr_pathpoint / norm(dr_pathpoint);
//             auto n_pp = {-t_pp.y, t_pp.x};
//             zombie.impulse += t_pp * m_seek_multiplier - zombie.vel;

//             utils::Vector2f dr_to_target = zombie.path_point - zombie.pos;
//             if (norm(dr_to_target) < 2 * zombie.radius || dot(dr_to_target, zombie.next_path_point - zombie.path_point) >= 0)
//             {
//                 // zombie.path_point = zombie.next_path_point;
//                 path = m_pathfinder->doPathFinding(zombie.pos, zombie.target, zombie.radius);
//                 zombie.path_point = path.path[1];
//                 if (vequal(zombie.path_point, zombie.target))
//                 {
//                     zombie.next_path_point = zombie.target;
//                 }
//                 else
//                 {
//                     zombie.next_path_point = path.path[2];
//                 }
//             }
//             zombie.target = m_player.pos; // rayCast(zombie.pos, angle2dir(randf(0, 360)), m_map->box_size_.x / 2.f, *m_cdt);

//             // zombie.impulse += m_seek_multiplier * dr_to_target / norm(dr_to_target) - zombie.vel;
//             truncate(zombie.impulse, m_max_force);
//             truncate(zombie.vel, m_max_vel);
//             i++;
//         }

//         applyWallConstraint();

//         i = 0;
//         for (auto &zombie : m_zombies.getObjects())
//         {
//             zombie.vel += zombie.impulse * dt + zombie.impulse_vel;
//             zombie.impulse_vel -= zombie.impulse_vel * m_friction;
//             zombie.pos += zombie.vel * dt;
//             if (!isInMap(zombie.pos))
//             {
//                 zombie.pos -= 1.1f * zombie.vel * dt;
//             }
//             zombie.impulse *= 0.f;
//             m_searcher.move(zombie.pos, i);
//             i++;
//         }
//     }

//     m_player.pos += m_player.vel;
// }

// void Game::smoothLights(const sf::RenderTexture &light_texture, sf::RenderTarget &scene)
// {
//     sf::VertexArray m_texture_rect;
//     m_texture_rect.setPrimitiveType(sf::Quads);
//     m_texture_rect.resize(4);

//     auto old_view = scene.getView();
//     scene.setView(scene.getDefaultView());

//     utils::Vector2f rect_size;
//     rect_size.x = m_texture_pass[0].getSize().x;
//     rect_size.y = m_texture_pass[0].getSize().y;
//     m_texture_rect[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 1}};
//     m_texture_rect[1] = sf::Vertex{{rect_size.x, 0}, sf::Color::Transparent, {1, 1}};
//     m_texture_rect[2] = sf::Vertex{{rect_size.x, rect_size.y}, sf::Color::Transparent, {1, 0}};
//     m_texture_rect[3] = sf::Vertex{{0, rect_size.y}, sf::Color::Transparent, {0, 0}};

//     sf::RenderStates states;
//     states.shader = &m_light_smoother;
//     states.blendMode = sf::BlendNone;
//     m_light_smoother.setUniform("image", m_light_texture.getTexture());

//     m_texture_pass[0].draw(m_texture_rect, states);
//     m_texture_pass[0].display();
//     for (int i = 0; i < 5; ++i)
//     {
//         m_light_smoother.setUniform("vertical", false);
//         m_light_smoother.setUniform("image", m_texture_pass[0].getTexture());
//         m_texture_pass[1].draw(m_texture_rect, states);
//         m_texture_pass[1].display();

//         m_light_smoother.setUniform("vertical", true);
//         m_light_smoother.setUniform("image", m_texture_pass[1].getTexture());
//         m_texture_pass[0].draw(m_texture_rect, states);
//         m_texture_pass[0].display();
//     }

//     rect_size.x = scene.getSize().x;
//     rect_size.y = scene.getSize().y;

//     m_texture_rect[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 1}};
//     m_texture_rect[1] = sf::Vertex{{rect_size.x, 0}, sf::Color::Transparent, {1, 1}};
//     m_texture_rect[2] = sf::Vertex{{rect_size.x, rect_size.y}, sf::Color::Transparent, {1, 0}};
//     m_texture_rect[3] = sf::Vertex{{0, rect_size.y}, sf::Color::Transparent, {0, 0}};

//     m_full_pass.setUniform("image", m_texture_pass[0].getTexture());

//     states.blendMode = sf::BlendMultiply;
//     states.shader = &m_full_pass;
//     m_texture_combine.draw(m_texture_rect, states);

//     scene.draw(m_texture_rect, states);
//     scene.setView(old_view);
// }

// static int n_visited = 0;

// void Game::draw(sf::RenderWindow &window)
// {
//     m_map->draw(window);

//     for (auto s : m_shots)
//     {
//         drawLine(window, s.from, s.to);
//     }

//     for (auto &zombie : m_zombies.getObjects())
//     {
//         sf::CircleShape c;
//         c.setRadius(1.f);
//         c.setFillColor(sf::Color::Red);
//         c.setPosition(zombie.pos);
//         window.draw(c);
//     }

//     auto &triangles = m_cdt->triangles_;
//     int tri_ind = 0;
//     for (auto &triangle : triangles)
//     {
//         auto p1 = static_cast<utils::Vector2f>(triangle.verts[0]);
//         auto p2 = static_cast<utils::Vector2f>(triangle.verts[1]);
//         auto p3 = static_cast<utils::Vector2f>(triangle.verts[2]);
//         auto &edge_transparency = triangle.is_transparent;
//         sf::Color color1 = edge_transparency[0] ? sf::Color::Transparent : sf::Color::Yellow;
//         drawLine(window, p1, p2, color1);
//         sf::Color color2 = edge_transparency[1] ? sf::Color::Transparent : sf::Color::Yellow;
//         drawLine(window, p2, p3, color2);
//         sf::Color color3 = edge_transparency[2] ? sf::Color::Transparent : sf::Color::Yellow;
//         drawLine(window, p3, p1, color3);

//         // drawTriInd(window, triangle, tri_ind);
//         tri_ind++;
//     }

//     if (m_changing_map)
//     {
//         if (m_selected_tri_1 != -1)
//         {
//             drawTriangle(window, m_cdt->triangles_.at(m_selected_tri_1), sf::Color({0, 0, 255, 125}));
//         }
//         if (m_selected_tri_2 != -1)
//         {
//             drawTriangle(window, m_cdt->triangles_.at(m_selected_tri_2), sf::Color({0, 0, 255, 125}));
//         }
//     }

//     m_player.draw(window);
//     m_player_vision->contrstuctField(m_player.pos, angle2dir(m_player.angle));

//     auto toc = std::chrono::high_resolution_clock::now();
//     auto rays2 = findVisionField2(m_player.pos, m_vision_distance, *m_cdt);
//     auto vertices2 = m_player_vision->getDrawVertices(); // verticesFromRays3(m_player.pos, rays2);
//     std::cout << "raycasting2 took: "
//               << std::chrono::duration_cast<std::chrono::microseconds>(toc - std::chrono::high_resolution_clock::now())
//               << "\n";
//     toc = std::chrono::high_resolution_clock::now();

//     sf::Color base_color = {0, 0, 0, 255};
//     m_light_texture.setView(window.getView());
//     m_light_texture.clear(base_color);
//     m_light_texture.draw(vertices2);
//     m_light_texture.display();

//     if (m_lights_on)
//     {
//         m_light_cut.setView(window.getView());
//         m_light_cut.clear({0, 0, 0, 255});

//         sf::VertexArray circle_verts;
//         circle_verts.resize(50 + 2);
//         circle_verts.setPrimitiveType(sf::TriangleFan);
//         sf::Color color(m_light_color[0] * 255, m_light_color[1] * 255, m_light_color[2] * 255, 255);
//         circle_verts[0] = {m_player.pos, color};
//         for (auto i = 0; i < 50; ++i)
//         {
//             float angle = i / 50. * 360.f;
//             utils::Vector2f r = m_player.pos + m_vision_distance * angle2dir(angle);
//             circle_verts[i + 1].position = r;
//             circle_verts[i + 1].color = base_color;
//             circle_verts[i + 1].color.a = 0;
//         }
//         circle_verts[50 + 1] = circle_verts[1];

//         sf::VertexArray circle_verts2;
//         circle_verts2.resize(50 + 2);
//         circle_verts2.setPrimitiveType(sf::TriangleFan);
//         circle_verts2[0] = {m_player.pos + utils::Vector2f(m_vision_distance, 0), sf::Color::Green};
//         for (auto i = 0; i < 50; ++i)
//         {
//             float angle = i / 50. * 360.f;
//             utils::Vector2f r = circle_verts2[0].position + m_vision_distance * angle2dir(angle);
//             circle_verts2[i + 1].position = r;
//             circle_verts2[i + 1].color = base_color;
//             circle_verts2[i + 1].color.a = 0;
//         }
//         circle_verts2[50 + 1] = circle_verts2[1];
//         m_light_cut.setView(window.getView());
//         sf::RenderStates states;
//         states.blendMode.colorSrcFactor = sf::BlendMode::SrcColor;
//         states.blendMode.colorDstFactor = sf::BlendMode::DstColor;
//         states.blendMode.colorEquation = sf::BlendMode::Add;
//         // m_light_cut.draw(circle_verts2, states);
//         m_light_cut.draw(circle_verts, states);
//         m_light_cut.display();
//         //
//         combineTextures(m_light_cut, m_light_texture, m_full_pass, sf::BlendMultiply);
//         m_light_texture.display();

//         smoothLights(m_light_texture, window);
//     }

//     drawUI(window);

//     for (int i = 1; i < m_pap.path.size(); ++i)
//     {
//         drawLine(m_window, m_pap.path[i], m_pap.path[i - 1], m_point_visible ? sf::Color::Yellow : sf::Color::Green);
//     }
// }

// void Game::drawUI(sf::RenderWindow &window)
// {

//     ImGui::SFML::Update(window, m_clock.restart());
//     ImGui::Begin("Rays");
//     ImGui::InputInt("Rays count", &m_num_rays, 1, 20);
//     ImGui::SliderFloat("Vision Distance", &m_vision_distance, 0, 500);
//     if (ImGui::Button("Toggle Lights"))
//     {
//         m_lights_on = !m_lights_on;
//     }

//     ImGui::ColorPicker3("Light Color", m_light_color);
//     ImGui::End();

//     ImGui::Begin("Physics");
//     ImGui::SliderFloat("Repulsion", &m_repulse_multiplier, 0, 1000);
//     ImGui::SliderFloat("Seek", &m_seek_multiplier, 0, 1000);
//     ImGui::SliderFloat("Max Force", &m_max_force, 0, 1000);
//     ImGui::SliderFloat("Max Vel", &m_max_vel, 0, 1000);
//     ImGui::SliderFloat("Shot force", &m_hit_force, 0, 1000);
//     ImGui::SliderFloat("Friction", &m_friction, 0, 1);
//     ImGui::End();

//     ImGui::Begin("Map Editor");
//     if (ImGui::Button("Change Map"))
//     {
//         m_changing_map = !m_changing_map;
//     }
//     if (ImGui::Button("Toggle Transparency"))
//     {
//         m_cdt->toggleTransparency(m_selected_tri_1, m_selected_tri_2);
//     }

//     ImGui::End();
//     ImGui::SFML::Render(m_window);
// }

// utils::Vector2f Game::rayCast(utils::Vector2f from, utils::Vector2f dir, float max_length, Triangulation &cdt)
// {

//     auto start_tri_ind = cdt.findTriangle(from, false);

//     auto to = from + dir * 10000.f;
//     std::vector<std::pair<TriInd, TriInd>> to_visit = {{start_tri_ind, -1}};

//     float best_dist = 0.f;
//     utils::Vector2f best_hit = to;
//     while (!to_visit.empty())
//     {
//         auto current_tri_ind = to_visit.back().first;
//         auto prev_tri_ind = to_visit.back().second;
//         to_visit.pop_back();
//         n_visited++;

//         const auto &current_tri = cdt.triangles_.at(current_tri_ind);

//         bool segment_intersected = false;
//         for (int i = 0; i < 3; ++i)
//         {
//             if (current_tri.neighbours[i] == prev_tri_ind)
//             {
//                 continue; //! we dont look from where we came from;
//             }
//             auto v1 = asFloat(current_tri.verts[i]);
//             auto v2 = asFloat(current_tri.verts[next(i)]);
//             auto dv21 = v2 - v1;
//             utils::Vector2f norm = {-dv21.y, dv21.x};
//             utils::Vector2f hit;
//             auto cos_a = dot(norm, dir);
//             if (segmentsIntersect(v1, v2, from, to, hit))
//             {
//                 if (current_tri.is_constrained[i])
//                 {
//                     auto hit_dist = dist(from, hit);
//                     if (hit_dist > best_dist)
//                     {
//                         best_dist = hit_dist;
//                         best_hit = hit;
//                         if (best_dist > max_length)
//                         {
//                             return from + dir * max_length;
//                         }
//                         return hit;
//                     }
//                 }

//                 if (!current_tri.is_constrained[i] && cos_a >= 0)
//                 {
//                     to_visit.emplace_back(current_tri.neighbours[i], current_tri_ind);
//                     break;
//                 }
//             }
//         }
//     }
//     std::cout << "visited: " << n_visited << " triangles\n";
//     return from + dir * max_length;
// }

// struct RayInfo
// {
//     Edgef edge;
//     Game::RayOrientation orient;

//     RayInfo(utils::Vector2f r1, utils::Vector2f r2, Game::RayOrientation orient)
//         : edge(r1, r2), orient(orient) {}
// };

// std::vector<Game::RayInfo>
// Game::findVisionField(utils::Vector2f from, float max_length, Triangulation &cdt)
// {

//     const auto &triangles = cdt.triangles_;
//     auto start_tri_ind = cdt.findTriangle(from, false);

//     // std::vector<Edgef> vision_field;
//     std::vector<RayInfo> vision_field;
//     struct Walker
//     {
//         TriInd prev_tri_ind;
//         TriInd curr_tri_ind;
//         utils::Vector2f left;
//         utils::Vector2f right;
//     };

//     std::vector<Walker> to_visit;
//     auto &curr_tri = triangles.at(start_tri_ind);
//     for (int i = 0; i < 3; ++i)
//     {
//         if (!curr_tri.is_constrained[i])
//         {
//             to_visit.push_back({start_tri_ind,
//                                 curr_tri.neighbours[i],
//                                 asFloat(curr_tri.verts[next(i)]),
//                                 asFloat(curr_tri.verts[i])});
//         }
//         // else
//         // {
//         vision_field.emplace_back(from, asFloat(curr_tri.verts[i]), RayOrientation::RIGHT);
//         vision_field.emplace_back(from, asFloat(curr_tri.verts[next(i)]), RayOrientation::LEFT);
//         // vision_field.emplace_back(from, asFloat(curr_tri.verts[next(i)]));
//         // }
//     }

//     while (!to_visit.empty())
//     {

//         auto curr_tri_ind = to_visit.back().curr_tri_ind;
//         auto prev_tri_ind = to_visit.back().prev_tri_ind;
//         auto left = to_visit.back().left;
//         auto right = to_visit.back().right;
//         auto &curr_tri = triangles.at(curr_tri_ind);
//         to_visit.pop_back();

//         if (dist(left, from) > max_length && dist(right, from) > max_length)
//         {
//             auto end_left = from + max_length * (left - from) / norm(left - from);
//             auto end_right = from + max_length * (right - from) / norm(right - from);
//             vision_field.emplace_back(from, end_left, RayOrientation::LEFT);
//             vision_field.emplace_back(from, end_right, RayOrientation::RIGHT);
//             continue;
//         }
//         if (orient2(from, right, left) < 0.f)
//         {
//             continue;
//         }

//         auto opposite_ind_in_tri = cdt.oppositeIndex(prev_tri_ind, curr_tri);
//         auto opposite_vert = asFloat(curr_tri.verts[opposite_ind_in_tri]);
//         auto left_vert = asFloat(curr_tri.verts[next(opposite_ind_in_tri)]);
//         auto right_vert = asFloat(curr_tri.verts[prev(opposite_ind_in_tri)]);

//         auto o_left = orient2(from, opposite_vert, left);
//         auto o_right = orient2(from, right, opposite_vert);

//         bool left_is_constrained = curr_tri.is_constrained[opposite_ind_in_tri];
//         bool right_is_constrained = curr_tri.is_constrained[prev(opposite_ind_in_tri)];
//         bool left_was_added = vequal(left_vert, left);
//         bool right_was_added = vequal(right_vert, right);

//         if (o_left >= 0.f && o_right >= 0.f) //! opposite vertex is in vision field
//         {

//             if (!left_is_constrained)
//             {
//                 to_visit.push_back({curr_tri_ind,
//                                     curr_tri.neighbours[opposite_ind_in_tri],
//                                     left,
//                                     opposite_vert});
//             }
//             else
//             {
//                 if (left_was_added)
//                 {
//                     vision_field.emplace_back(from, opposite_vert, RayOrientation::LEFT);
//                 }
//                 else
//                 {
//                     auto end_left = from + max_length * (left - from) / norm(left - from);
//                     utils::Vector2f new_left = end_left;
//                     segmentsIntersect(from, end_left, opposite_vert, left_vert, new_left);
//                     vision_field.emplace_back(from, new_left, RayOrientation::LEFT);
//                 }
//             }
//             if (!right_is_constrained)
//             {
//                 to_visit.push_back({curr_tri_ind,
//                                     curr_tri.neighbours[prev(opposite_ind_in_tri)],
//                                     opposite_vert,
//                                     right});
//             }
//             else
//             {
//                 if (right_was_added)
//                 {
//                     vision_field.emplace_back(from, opposite_vert, RayOrientation::RIGHT);
//                 }
//                 else
//                 {
//                     auto end_right = from + max_length * (right - from) / norm(right - from);
//                     utils::Vector2f new_right = end_right;
//                     segmentsIntersect(from, end_right, right_vert, opposite_vert, new_right);
//                     vision_field.emplace_back(from, new_right, RayOrientation::RIGHT);
//                 }
//             }
//             if (!right_is_constrained && !left_is_constrained) //! we hit crossroads
//             {
//                 vision_field.emplace_back(from, opposite_vert, RayOrientation::RIGHT);
//             }
//         }
//         else if (o_left < 0.f) //! oppposite vert is not visible and above left
//         {
//             auto end_left = from + max_length * (left - from) / norm(left - from);
//             utils::Vector2f new_left = end_left;
//             if (segmentsIntersect(from, end_left, right_vert, opposite_vert, new_left))
//             {
//             }
//             if (!right_is_constrained)
//             {
//                 to_visit.push_back({curr_tri_ind,
//                                     curr_tri.neighbours[prev(opposite_ind_in_tri)],
//                                     new_left,
//                                     right});
//             }
//             else
//             {
//                 vision_field.emplace_back(from, new_left, RayOrientation::LEFT);
//                 if (!right_was_added)
//                 {
//                     auto end_right = from + max_length * (right - from) / norm(right - from);
//                     utils::Vector2f new_right = end_right;
//                     segmentsIntersect(from, end_right, right_vert, opposite_vert, new_right);
//                     vision_field.emplace_back(from, new_right, RayOrientation::RIGHT);
//                 }
//             }
//         }
//         else if (o_right < 0.f) //! oppposite vert is not visible and below right
//         {
//             auto end_right = from + max_length * (right - from) / norm(right - from);
//             utils::Vector2f new_right = end_right;
//             if (segmentsIntersect(from, end_right, left_vert, opposite_vert, new_right))
//             {
//             }
//             if (!left_is_constrained)
//             {
//                 to_visit.push_back({curr_tri_ind,
//                                     curr_tri.neighbours[opposite_ind_in_tri],
//                                     left,
//                                     new_right});
//             }
//             else
//             {
//                 vision_field.emplace_back(from, new_right, RayOrientation::RIGHT);
//                 if (!left_was_added)
//                 {
//                     auto end_left = from + max_length * (left - from) / norm(left - from);
//                     utils::Vector2f new_left = end_left;
//                     segmentsIntersect(from, end_left, left_vert, opposite_vert, new_left);
//                     vision_field.emplace_back(from, new_left, RayOrientation::LEFT);
//                 }
//             }
//         }
//         else
//         {
//             throw std::runtime_error("floating point error in orientation calculation!");
//         }
//     }
//     return vision_field;
// }

// std::vector<VisionCone>
// Game::findVisionField2(utils::Vector2f from, float max_length, Triangulation &cdt)
// {

//     utils::Vector2f left_limit = from + angle2dir(-45);
//     utils::Vector2f right_limit = from + angle2dir(45);

//     const auto &triangles = cdt.triangles_;
//     auto start_tri_ind = cdt.findTriangle(from, false);

//     std::vector<VisionCone> vision_field;
//     struct Walker
//     {
//         TriInd prev_tri_ind;
//         TriInd curr_tri_ind;
//         utils::Vector2f left;
//         utils::Vector2f right;
//     };

//     std::vector<Walker> to_visit;
//     auto &curr_tri = triangles.at(start_tri_ind);
//     for (int i = 0; i < 3; ++i)
//     {
//         vision_field.emplace_back();
//         int cone_ind = vision_field.size() - 1;
//         auto left = asFloat(curr_tri.verts[next(i)]);
//         auto right = asFloat(curr_tri.verts[i]);
//         if (!curr_tri.is_constrained[i])
//         {
//             to_visit.push_back({
//                 start_tri_ind,
//                 curr_tri.neighbours[i],
//                 left,
//                 right,
//             });
//         }
//         else
//         {
//             vision_field.emplace_back(left, right);
//         }
//     }

//     while (!to_visit.empty())
//     {

//         auto curr_tri_ind = to_visit.back().curr_tri_ind;
//         auto prev_tri_ind = to_visit.back().prev_tri_ind;
//         auto left = to_visit.back().left;
//         auto right = to_visit.back().right;
//         auto &curr_tri = triangles.at(curr_tri_ind);
//         to_visit.pop_back();

//         if (dist(left, from) > max_length && dist(right, from) > max_length)
//         {
//             auto end_left = from + max_length * (left - from) / norm(left - from);
//             auto end_right = from + max_length * (right - from) / norm(right - from);
//             vision_field.emplace_back(end_left, end_right);
//             continue;
//         }
//         if (orient2(from, right, left) < 0.f) //! we do not see anything anymore
//         {
//             continue;
//         }

//         auto opposite_ind_in_tri = cdt.oppositeIndex(prev_tri_ind, curr_tri);
//         auto opposite_vert = asFloat(curr_tri.verts[opposite_ind_in_tri]);
//         auto left_vert = asFloat(curr_tri.verts[next(opposite_ind_in_tri)]);
//         auto right_vert = asFloat(curr_tri.verts[prev(opposite_ind_in_tri)]);

//         auto o_left = orient2(from, opposite_vert, left);
//         auto o_right = orient2(from, right, opposite_vert);

//         bool left_is_constrained = curr_tri.is_constrained[opposite_ind_in_tri];
//         bool right_is_constrained = curr_tri.is_constrained[prev(opposite_ind_in_tri)];
//         bool left_was_added = vequal(left_vert, left);
//         bool right_was_added = vequal(right_vert, right);

//         if (o_left >= 0.f && o_right >= 0.f) //! opposite vertex is in vision field
//         {

//             if (!left_is_constrained)
//             {
//                 to_visit.push_back({curr_tri_ind,
//                                     curr_tri.neighbours[opposite_ind_in_tri],
//                                     left,
//                                     opposite_vert});
//             }
//             else
//             {
//                 if (left_was_added)
//                 {
//                     vision_field.emplace_back(left, opposite_vert);
//                 }
//                 else
//                 {
//                     auto end_left = from + max_length * (left - from) / norm(left - from);
//                     utils::Vector2f new_left = end_left;
//                     segmentsIntersect(from, end_left, opposite_vert, left_vert, new_left);
//                     vision_field.emplace_back(new_left, opposite_vert);
//                 }
//             }
//             if (!right_is_constrained)
//             {
//                 to_visit.push_back({curr_tri_ind,
//                                     curr_tri.neighbours[prev(opposite_ind_in_tri)],
//                                     opposite_vert,
//                                     right});
//             }
//             else
//             {
//                 if (right_was_added)
//                 {
//                     vision_field.emplace_back(opposite_vert, right);
//                 }
//                 else
//                 {
//                     auto end_right = from + max_length * (right - from) / norm(right - from);
//                     utils::Vector2f new_right = end_right;
//                     segmentsIntersect(from, end_right, right_vert, opposite_vert, new_right);
//                     vision_field.emplace_back(opposite_vert, new_right);
//                 }
//             }
//         }
//         else if (o_left < 0.f) //! oppposite vert is not visible and above left
//         {
//             auto end_left = from + max_length * (left - from) / norm(left - from);
//             utils::Vector2f new_left = end_left;
//             if (segmentsIntersect(from, end_left, right_vert, opposite_vert, new_left))
//             {
//             }
//             if (!right_is_constrained)
//             {
//                 to_visit.push_back({curr_tri_ind,
//                                     curr_tri.neighbours[prev(opposite_ind_in_tri)],
//                                     new_left,
//                                     right});
//             }
//             else
//             {
//                 if (!right_was_added)
//                 {
//                     auto end_right = from + max_length * (right - from) / norm(right - from);
//                     utils::Vector2f new_right = end_right;
//                     segmentsIntersect(from, end_right, right_vert, opposite_vert, new_right);
//                     vision_field.emplace_back(new_left, new_right);
//                 }
//                 else
//                 {
//                     vision_field.emplace_back(new_left, right);
//                 }
//             }
//         }
//         else if (o_right < 0.f) //! oppposite vert is not visible and below right
//         {
//             auto end_right = from + max_length * (right - from) / norm(right - from);
//             utils::Vector2f new_right = end_right;
//             if (segmentsIntersect(from, end_right, left_vert, opposite_vert, new_right))
//             {
//             }
//             if (!left_is_constrained)
//             {
//                 to_visit.push_back({curr_tri_ind,
//                                     curr_tri.neighbours[opposite_ind_in_tri],
//                                     left,
//                                     new_right});
//             }
//             else
//             {
//                 if (!left_was_added)
//                 {
//                     auto end_left = from + max_length * (left - from) / norm(left - from);
//                     utils::Vector2f new_left = end_left;
//                     segmentsIntersect(from, end_left, left_vert, opposite_vert, new_left);
//                     vision_field.emplace_back(new_left, new_right);
//                 }
//                 else
//                 {
//                     vision_field.emplace_back(left, new_right);
//                 }
//             }
//         }
//         else
//         {
//             throw std::runtime_error("floating point error in orientation calculation!");
//         }
//     }
//     return vision_field;
// }

// sf::VertexArray Game::verticesFromRays(std::vector<Edgef> &rays)
// {
//     auto angle_dist_cond = [](Edgef &r1, Edgef &r2)
//     {
//         auto a1 = dir2angle(r1.t);
//         auto a2 = dir2angle(r2.t);
//         if (approx_equal(a1, a2))
//         {
//             return r1.l > r2.l;
//         }
//         return a1 < a2;
//     };
//     std::sort(rays.begin(), rays.end(), angle_dist_cond);
//     for (auto &ray : rays)
//     {
//     }

//     int n_rays = rays.size();
//     sf::VertexArray vertices;
//     vertices.setPrimitiveType(sf::TriangleFan);
//     int n_interpol = 1;
//     vertices.resize(n_interpol * n_rays + 2);
//     vertices[0].position = m_player.pos;
//     vertices[0].color = sf::Color(m_light_color[0] * 255,
//                                   m_light_color[1] * 255,
//                                   m_light_color[2] * 255, 255);
//     for (int i = 0; i < n_rays; ++i)
//     {
//         auto this_ray = rays[i];
//         auto next_ray = rays[(i + 1) % n_rays];

//         auto this_ray_pos = this_ray.to();
//         auto next_ray_pos = next_ray.to();

//         for (int k = 0; k < n_interpol; ++k)
//         {
//             auto pos = ((float)(n_interpol - k) * this_ray_pos + (float)k * next_ray_pos) / (float)n_interpol;
//             ;
//             float len = ((float)(n_interpol - k) * this_ray.l + (float)k * next_ray.l) / (float)n_interpol;
//             ;
//             float factor = 1 - len / m_vision_distance;
//             vertices[n_interpol * i + k + 1].position = pos;
//             vertices[n_interpol * i + k + 1].color = sf::Color(m_light_color[0] * 255,
//                                                                m_light_color[1] * 255,
//                                                                m_light_color[2] * 255,
//                                                                (factor) * 255);
//         }
//     }
//     vertices[n_interpol * n_rays + 1] = vertices[1];
//     return vertices;
// }

// sf::VertexArray Game::verticesFromRays2(std::vector<RayInfo> &rays)
// {
//     auto angle_dist_cond = [](RayInfo &r1, RayInfo &r2)
//     {
//         auto a1 = dir2angle(r1.edge.t);
//         auto a2 = dir2angle(r2.edge.t);
//         if (approx_equal(a1, a2) && r1.orient == RayOrientation::LEFT)
//         {
//             return r1.edge.l < r2.edge.l;
//         }
//         if (approx_equal(a1, a2) && r1.orient == RayOrientation::RIGHT)
//         {
//             return r1.edge.l > r2.edge.l;
//         }
//         return a1 < a2;
//     };
//     std::sort(rays.begin(), rays.end(), angle_dist_cond);
//     for (auto &ray : rays)
//     {
//     }

//     int n_rays = rays.size();
//     sf::VertexArray vertices;
//     vertices.setPrimitiveType(sf::TriangleFan);
//     int n_interpol = 1;
//     vertices.resize(n_interpol * n_rays + 2);
//     vertices[0].position = m_player.pos;
//     vertices[0].color = sf::Color(m_light_color[0] * 255,
//                                   m_light_color[1] * 255,
//                                   m_light_color[2] * 255, 255);
//     for (int i = 0; i < n_rays; ++i)
//     {
//         auto this_ray = rays[i].edge;
//         auto next_ray = rays[(i + 1) % n_rays].edge;

//         auto this_ray_pos = this_ray.to();
//         auto next_ray_pos = next_ray.to();

//         for (int k = 0; k < n_interpol; ++k)
//         {
//             auto pos = ((float)(n_interpol - k) * this_ray_pos + (float)k * next_ray_pos) / (float)n_interpol;
//             ;
//             float len = ((float)(n_interpol - k) * this_ray.l + (float)k * next_ray.l) / (float)n_interpol;
//             ;
//             float factor = 1 - len / m_vision_distance;
//             vertices[n_interpol * i + k + 1].position = pos;
//             vertices[n_interpol * i + k + 1].color = sf::Color(m_light_color[0] * 255,
//                                                                m_light_color[1] * 255,
//                                                                m_light_color[2] * 255,
//                                                                (factor) * 255);
//         }
//     }
//     vertices[n_interpol * n_rays + 1] = vertices[1];
//     return vertices;
// }

// sf::VertexArray Game::verticesFromRays3(utils::Vector2f from, std::vector<VisionCone> &rays)
// {
//     auto angle_dist_cond = [from](VisionCone &r1, VisionCone &r2)
//     {
//         auto a1 = dir2angle(r1.left - from);
//         auto a2 = dir2angle(r2.left - from);
//         return a1 < a2;
//     };
//     std::sort(rays.begin(), rays.end(), angle_dist_cond);

//     int n_rays = rays.size();
//     sf::VertexArray vertices;
//     vertices.setPrimitiveType(sf::Triangles);
//     vertices.resize(3 * n_rays);

//     sf::Vertex center_vert;
//     center_vert.position = m_player.pos;
//     center_vert.color = sf::Color(m_light_color[0] * 255,
//                                   m_light_color[1] * 255,
//                                   m_light_color[2] * 255, 255);
//     for (int i = 0; i < n_rays; ++i)
//     {
//         Edgef left_ray = {from, rays[i].left};
//         Edgef right_ray = {from, rays[i].right};

//         auto left_ray_pos = left_ray.to();
//         auto right_ray_pos = right_ray.to();

//         float factor_left = 1 - left_ray.l / m_vision_distance;
//         float factor_right = 1 - right_ray.l / m_vision_distance;
//         vertices[3 * i + 0] = center_vert;

//         vertices[3 * i + 1].position = left_ray_pos;
//         vertices[3 * i + 1].color = sf::Color(m_light_color[0] * 255,
//                                               m_light_color[1] * 255,
//                                               m_light_color[2] * 255,
//                                               (factor_left) * 255);

//         vertices[3 * i + 2].position = right_ray_pos;
//         vertices[3 * i + 2].color = sf::Color(m_light_color[0] * 255,
//                                               m_light_color[1] * 255,
//                                               m_light_color[2] * 255,
//                                               (factor_right) * 255);
//     }
//     return vertices;
// }

// std::vector<Edgef> Game::findVisionFieldBasic(utils::Vector2f from, float max_length)
// {
//     auto tic = std::chrono::high_resolution_clock::now();
//     auto closest_edges = m_map->p_edges_->calcContactEdgesO(m_player.pos, m_vision_distance);
//     auto toc = std::chrono::high_resolution_clock::now();
//     std::cout << "finding edges took: " << std::chrono::duration_cast<std::chrono::microseconds>(toc - tic)
//               << "\n";

//     std::vector<Edgef> rays;
//     for (int i = 0; i < 4; ++i)
//     {
//         for (auto &edge : closest_edges[i])
//         {
//             utils::Vector2f n = {edge.t.y, -edge.t.x};
//             auto dr1 = edge.from - m_player.pos;
//             auto dr2 = edge.to() - m_player.pos;
//             utils::Vector2f x = (edge.from + edge.to()) / 2.f + n * 10.f;
//             // drawLine(window, (edge.from + edge.to()) / 2.f, x);
//             if (dot(n, dr1) < 0)
//             {
//                 if (dist(edge.from, m_player.pos) < m_vision_distance)
//                 {
//                     auto t = edge.from - m_player.pos;
//                     auto a0 = dir2angle(t);
//                     utils::Vector2f end_point1 = rayCast(m_player.pos, angle2dir(a0 - 0.1f), m_vision_distance, *m_cdt);
//                     utils::Vector2f end_point2 = rayCast(m_player.pos, angle2dir(a0 + 0.1f), m_vision_distance, *m_cdt);
//                     rays.push_back({m_player.pos, end_point1});
//                     rays.push_back({m_player.pos, end_point2});
//                     // drawLine(window, m_player.pos, end_point1);
//                     // drawLine(window, m_player.pos, end_point2);
//                 }
//             }

//             if (dot(n, dr2) < 0)
//             {
//                 if (dist(edge.to(), m_player.pos) < m_vision_distance)
//                 {
//                     auto t = edge.to() - m_player.pos;
//                     auto a0 = dir2angle(t);
//                     utils::Vector2f end_point1 = rayCast(m_player.pos, angle2dir(a0 - 0.1f), m_vision_distance, *m_cdt);
//                     utils::Vector2f end_point2 = rayCast(m_player.pos, angle2dir(a0 + 0.1f), m_vision_distance, *m_cdt);
//                     rays.push_back({m_player.pos, end_point1});
//                     rays.push_back({m_player.pos, end_point2});
//                     // drawLine(window, m_player.pos, end_point1);
//                     // drawLine(window, m_player.pos, end_point2);
//                 }
//             }
//         }
//     }
//     for (int i = 0; i < m_num_rays; ++i)
//     {
//         float angle = 180 + i * 360.f / m_num_rays;
//         auto dir = angle2dir(angle);
//         utils::Vector2f end_point = rayCast(m_player.pos, dir, m_vision_distance, *m_cdt);
//         rays.push_back({m_player.pos, end_point});
//         // drawLine(window, m_player.pos, rays.back().to(), sf::Color::Red);
//     }
//     auto vertices = verticesFromRays(rays);

//     std::cout << "visited: " << (float)n_visited / rays.size() << " triangles\n";
//     n_visited = 0;
//     std::cout << "raycasting took: "
//               << std::chrono::duration_cast<std::chrono::microseconds>(toc - std::chrono::high_resolution_clock::now())
//               << "\n";
//     toc = std::chrono::high_resolution_clock::now();
//     return rays;
// }
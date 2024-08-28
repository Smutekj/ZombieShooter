// #ifndef BOIDS_GAME_H
// #define BOIDS_GAME_H

// #include <numeric>
// #include <fstream>
// #include <unordered_set>

// #include <Grid.h>
// #include "Utils/RandomTools.h"
// #include "Utils/ObjectPool.h"

// #include "core.h"
// #include <Particles.h>

// #include "Menu/GameState.h"
// #include "PathFinding/Triangulation.h"
// #include "PathFinding/PathFinder.h"
// #include "MapGrid.h"
// #include "GridNeighbourSearcher.h"
// #include "BVH.h"

// class UI;

// struct Zombie
// {
//     sf::Vector2f pos;
//     sf::Vector2f vel;
//     sf::Vector2f impulse_vel;
//     sf::Vector2f impulse;

//     sf::Vector2f path_point = {-1, -1};
//     sf::Vector2f next_path_point = {-1, -1};
//     sf::Vector2f target;
//     float radius;

// };


// enum class Context
// {
//     MAINMENU,
//     SETTINGS,
//     HIGHSCORE,
//     GAME
// };

// class ScenePostProcessor
// {

// public:
//     ScenePostProcessor()
//     {
//         m_texture_rect.resize(4);
//         m_texture_rect.setPrimitiveType(sf::Quads);
//     }

//     virtual void process(const sf::RenderTarget &source, sf::RenderTarget &destination) = 0;

// protected:
//     void setTextureSize(sf::Vector2u texture_size)
//     {
//         sf::Vector2f texture_sizef(texture_size.x, texture_size.y);
//         m_texture_rect[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 1}};
//         m_texture_rect[1] = sf::Vertex{{texture_sizef.x, 0}, sf::Color::Transparent, {1, 1}};
//         m_texture_rect[2] = sf::Vertex{{texture_sizef.x, texture_sizef.y}, sf::Color::Transparent, {1, 0}};
//         m_texture_rect[3] = sf::Vertex{{0, texture_sizef.y}, sf::Color::Transparent, {0, 0}};
//     }

// protected:
//     sf::VertexArray m_texture_rect;
//     ShaderHolder m_shaders;
// };

// class EdgeSmoother : public ScenePostProcessor
// {
// public:
//     EdgeSmoother() : ScenePostProcessor()
//     {
//         m_shaders.load(Shaders::ID::GaussianBlurPass, "../Resources/basic.vert", "../Resources/lightSmootherRGB.frag");
//         m_shaders.load(Shaders::ID::GaussianBlurPass, "../Resources/basic.vert", "../Resources/fullpass.frag");
//     }

//     virtual void process(const sf::RenderTexture &source, sf::RenderTarget &destination)
//     {
//         auto old_view = destination.getView();
//         destination.setView(destination.getDefaultView());

//         sf::Vector2f rect_size;
//         setTextureSize(m_texture_pass[0].getSize());

//         sf::RenderStates states;
//         auto &light_smoother = m_shaders.get(Shaders::ID::GaussianBlurPass);
//         ;
//         states.shader = &light_smoother;
//         states.blendMode = sf::BlendNone;
//         light_smoother.setUniform("image", source.getTexture());

//         m_texture_pass[0].draw(m_texture_rect, states);
//         m_texture_pass[0].display();
//         for (int i = 0; i < 5; ++i)
//         {
//             light_smoother.setUniform("vertical", false);
//             light_smoother.setUniform("image", m_texture_pass[0].getTexture());
//             m_texture_pass[1].draw(m_texture_rect, states);
//             m_texture_pass[1].display();

//             light_smoother.setUniform("vertical", true);
//             light_smoother.setUniform("image", m_texture_pass[1].getTexture());
//             m_texture_pass[0].draw(m_texture_rect, states);
//             m_texture_pass[0].display();
//         }

//         setTextureSize(destination.getSize());

//         auto &full_pass = m_shaders.get(Shaders::ID::AddPass);
//         full_pass.setUniform("image", m_texture_pass[0].getTexture());

//         states.blendMode = sf::BlendMultiply;
//         states.shader = &full_pass;
//         destination.draw(m_texture_rect, states);
//         destination.setView(old_view);
//     }

// private:
//     sf::RenderTexture m_texture_pass[2];
//     sf::RenderTexture m_texture_combine;
// };

// struct VisionCone
// {
//     sf::Vector2f left;
//     sf::Vector2f right;
//     VisionCone() = default;
//     VisionCone(sf::Vector2f left, sf::Vector2f right)
//         : left(left), right(right) {}
// };

// class LightSystem
// {

// };

// class PointLight
// {

// private:
//     sf::Vector2f m_center;
//     float m_radius;
//     sf::Color m_color;
// };

// class SceneEffect
// {

// protected:
//     std::vector<std::unique_ptr<ScenePostProcessor>> m_processors;
// };

// class SmoothingEffect : public SceneEffect
// {

// public:
//     SmoothingEffect()
//     {
//         // auto m_
//     }
// };

// class VisionField
// {

//     struct Walker
//     {
//         TriInd prev_tri_ind;
//         TriInd curr_tri_ind;
//         sf::Vector2f left;
//         sf::Vector2f right;
//     };

// public:
//     VisionField(Triangulation &cdt) : m_cdt(cdt) {}

//     bool isVisible(sf::Vector2f query)
//     {
//         auto angle_query = dir2angle(query - m_center);
//         if (dist(query, m_center) > m_vision_dist)
//         {
//             return false;
//         }
//         // if (angle_query < m_min_angle || angle_query > m_max_angle)
//         // {
//         //     return false;
//         // }

//         for (const auto &[left, right] : m_vision)
//         {
//             auto l = orient2(m_center, query, left);
//             auto r = orient2(m_center, right, query);
//             auto x = orient2(m_center, right, left);
//             if (l >= 0.f && r >= 0.f && x >= 0.f)
//             {
//                 return true;
//             }
//         }
//         return false;
//     }

//     void contrstuctField(sf::Vector2f from, sf::Vector2f look_dir)
//     {
//         auto left_limit = from + m_vision_dist * angle2dir(dir2angle(look_dir) + m_min_angle);
//         auto right_limit = from + m_vision_dist * angle2dir(dir2angle(look_dir) + m_max_angle);
//         assert(orient2(from, right_limit, left_limit) > 0.f);

//         auto in_vision_cone = [from, left_limit, right_limit](sf::Vector2f point)
//         {
//             return orient2(from, point, left_limit) >= 0.f && orient2(from, right_limit, point) >= 0.f;
//         };
//         auto lies_on_line = [](sf::Vector2f point, sf::Vector2f &left_line, sf::Vector2f right_line)
//         {
//             return approx_equal_zero(cross(left_line - point, right_line - point));
//         };

//         m_center = from;
//         m_vision.clear();
//         float max_length = 1000.f;

//         const auto &triangles = m_cdt.triangles_;
//         auto start_tri_ind = m_cdt.findTriangle(from, false);

//         std::vector<Walker> to_visit;
//         auto &curr_tri = triangles.at(start_tri_ind);

//         //! when standing on some line,
//         //! we move the point very little towards triangle center
//         //!  so that we don't have to deal with it (dirty but maybe works)
//         auto v1 = asFloat(curr_tri.verts[0]);
//         auto v2 = asFloat(curr_tri.verts[1]);
//         auto v3 = asFloat(curr_tri.verts[2]);
//         auto center = sf::Vector2f(v1 + v2 + v3) / 3.f;
//         if (lies_on_line(from, v2, v1) || lies_on_line(from, v3, v2) || lies_on_line(from, v1, v3))
//         {
//             from = from + (center - from) / norm(center - from) * 0.001f;
//         }

//         for (int i = 0; i < 3; ++i)
//         {
//             int cone_ind = m_vision.size() - 1;
//             auto left = asFloat(curr_tri.verts[next(i)]);
//             auto right = asFloat(curr_tri.verts[i]);

//             // if (!in_vision_cone(left))
//             // { //! find where left side of vision cone hit the tri-edge
//             //     auto end_left = from + max_length * (left_limit - from) / norm(left_limit - from);
//             //     if (!segmentsIntersect(from, end_left, right, left, left))
//             //     {
//             //         continue;
//             //     }
//             // }
//             // if (!in_vision_cone(right))
//             // { //! find where right side of vision cone hit the tri-edge
//             //     auto end_right = from + max_length * (right_limit - from) / norm(right_limit - from);
//             //     if (!segmentsIntersect(from, end_right, right, left, right))
//             //     {
//             //         //! if there is no intersection the side is not visible
//             //         continue;
//             //     }
//             // }

//             if (curr_tri.is_transparent[i])
//             {
//                 to_visit.push_back({start_tri_ind, curr_tri.neighbours[i], left, right});
//             }
//             else
//             {
//                 m_vision.emplace_back(left, right);
//             }
//         }

//         while (!to_visit.empty())
//         {

//             auto curr_tri_ind = to_visit.back().curr_tri_ind;
//             auto prev_tri_ind = to_visit.back().prev_tri_ind;
//             auto left = to_visit.back().left;
//             auto right = to_visit.back().right;
//             auto &curr_tri = triangles.at(curr_tri_ind);
//             to_visit.pop_back();
//             if (dist(left, from) > max_length && dist(right, from) > max_length)
//             {
//                 auto end_left = from + max_length * (left - from) / norm(left - from);
//                 auto end_right = from + max_length * (right - from) / norm(right - from);
//                 m_vision.emplace_back(end_left, end_right);
//                 continue;
//             }

//             if (orient2(from, right, left) < 0.f) //! we do not see anything anymore
//             {
//                 continue;
//             }

//             auto opposite_ind_in_tri = m_cdt.oppositeIndex(prev_tri_ind, curr_tri);
//             auto opposite_vert = asFloat(curr_tri.verts[opposite_ind_in_tri]);
//             auto left_vert = asFloat(curr_tri.verts[next(opposite_ind_in_tri)]);
//             auto right_vert = asFloat(curr_tri.verts[prev(opposite_ind_in_tri)]);

//             auto left_neighbour = curr_tri.neighbours[opposite_ind_in_tri];
//             auto right_neighbour = curr_tri.neighbours[prev(opposite_ind_in_tri)];

//             auto o_left = orient2(from, opposite_vert, left);
//             auto o_right = orient2(from, right, opposite_vert);

//             bool left_is_transparent = curr_tri.is_transparent[opposite_ind_in_tri];
//             bool right_is_transparent = curr_tri.is_transparent[prev(opposite_ind_in_tri)];
//             bool left_was_added = vequal(left_vert, left);
//             bool right_was_added = vequal(right_vert, right);

//             if (o_left >= 0.f && o_right >= 0.f) //! opposite vertex is in vision field
//             {

//                 if (left_is_transparent)
//                 {
//                     to_visit.push_back({curr_tri_ind, left_neighbour, left, opposite_vert});
//                 }
//                 else
//                 {
//                     if (left_was_added)
//                     {
//                         m_vision.emplace_back(left, opposite_vert);
//                     }
//                     else
//                     {
//                         auto end_left = from + max_length * (left - from) / norm(left - from);
//                         sf::Vector2f new_left = end_left;
//                         segmentsIntersect(from, end_left, opposite_vert, left_vert, new_left);
//                         m_vision.emplace_back(new_left, opposite_vert);
//                     }
//                 }
//                 if (right_is_transparent)
//                 {
//                     to_visit.push_back({curr_tri_ind, right_neighbour, opposite_vert, right});
//                 }
//                 else
//                 {
//                     if (right_was_added)
//                     {
//                         m_vision.emplace_back(opposite_vert, right);
//                     }
//                     else
//                     {
//                         auto end_right = from + max_length * (right - from) / norm(right - from);
//                         sf::Vector2f new_right = end_right;
//                         segmentsIntersect(from, end_right, right_vert, opposite_vert, new_right);
//                         m_vision.emplace_back(opposite_vert, new_right);
//                     }
//                 }
//             }
//             else if (o_left < 0.f) //! oppposite vert is not visible and above left
//             {
//                 auto end_left = from + max_length * (left - from) / norm(left - from);
//                 sf::Vector2f new_left = end_left;
//                 if (segmentsIntersect(from, end_left, right_vert, opposite_vert, new_left))
//                 {
//                 }
//                 if (right_is_transparent)
//                 {
//                     assert(!std::isnan(new_left.x) && !std::isnan(new_left.y));
//                     to_visit.push_back({curr_tri_ind, right_neighbour, new_left, right});
//                 }
//                 else
//                 {
//                     if (!right_was_added)
//                     {
//                         auto end_right = from + max_length * (right - from) / norm(right - from);
//                         sf::Vector2f new_right = end_right;
//                         segmentsIntersect(from, end_right, right_vert, opposite_vert, new_right);
//                         m_vision.emplace_back(new_left, new_right);
//                     }
//                     else
//                     {
//                         m_vision.emplace_back(new_left, right);
//                     }
//                 }
//             }
//             else if (o_right < 0.f) //! oppposite vert is not visible and below right
//             {
//                 auto end_right = from + max_length * (right - from) / norm(right - from);
//                 sf::Vector2f new_right = end_right;
//                 if (segmentsIntersect(from, end_right, left_vert, opposite_vert, new_right))
//                 {
//                 }
//                 if (left_is_transparent)
//                 {
//                     assert(!std::isnan(new_right.x) && !std::isnan(new_right.y));
//                     to_visit.push_back({curr_tri_ind, left_neighbour, left, new_right});
//                 }
//                 else
//                 {
//                     if (!left_was_added)
//                     {
//                         auto end_left = from + max_length * (left - from) / norm(left - from);
//                         sf::Vector2f new_left = end_left;
//                         segmentsIntersect(from, end_left, left_vert, opposite_vert, new_left);
//                         m_vision.emplace_back(new_left, new_right);
//                     }
//                     else
//                     {
//                         m_vision.emplace_back(left, new_right);
//                     }
//                 }
//             }
//             else
//             {
//                 throw std::runtime_error("floating point error in orientation calculation!");
//             }
//         }
//     }

//     sf::VertexArray getDrawVertices()
//     {

//         auto from = m_center;
//         auto angle_dist_cond = [from](VisionCone &r1, VisionCone &r2)
//         {
//             auto a1 = dir2angle(r1.left - from);
//             auto a2 = dir2angle(r2.left - from);
//             return a1 < a2;
//         };
//         std::sort(m_vision.begin(), m_vision.end(), angle_dist_cond);
//         int n_rays = m_vision.size();

//         sf::VertexArray vertices;
//         vertices.setPrimitiveType(sf::Triangles);
//         vertices.resize(3 * n_rays);

//         sf::Vertex center_vert;
//         center_vert.position = from;
//         center_vert.color = sf::Color(255, 255, 255, 255);
//         for (int i = 0; i < n_rays; ++i)
//         {

//             vertices[3 * i + 0] = center_vert;

//             vertices[3 * i + 1].position = m_vision[i].left;
//             vertices[3 * i + 1].color = sf::Color(255, 255, 255, 255);

//             vertices[3 * i + 2].position = m_vision[i].right;
//             vertices[3 * i + 2].color = sf::Color(255, 255, 255, 255);
//         }
//         return vertices;
//     }

// private:
//     sf::Vector2f m_center;
//     float m_vision_dist = 100.f;
//     float m_min_angle = -60;
//     float m_max_angle = +60;

//     std::vector<VisionCone> m_vision;
//     Triangulation &m_cdt;
// };

// struct GunShot
// {
//     sf::Vector2f from;
//     sf::Vector2f to;
// };

// class Game
// {

// public:
//     int score = 0;

//     enum class GameState
//     {
//         RUNNING,
//         WON,
//         PLAYER_DIED
//     };

//     enum class RayOrientation
//     {
//         LEFT,
//         RIGHT
//     };

//     struct RayInfo
//     {
//         Edgef edge;
//         Game::RayOrientation orient;

//         RayInfo(sf::Vector2f r1, sf::Vector2f r2, Game::RayOrientation orient)
//             : edge(r1, r2), orient(orient) {}
//     };

//     Game(sf::RenderWindow &window, KeyBindings &bindings);

//     void update(const float dt, sf::RenderWindow &win);

//     GameState getState() const
//     {
//         return state;
//     }

//     void handleEvent(const sf::Event &event);
//     void parseInput(sf::RenderWindow &window);
//     void draw(sf::RenderWindow &window);

// private:
//     void drawUI(sf::RenderWindow &window);
//     void moveView(sf::RenderWindow &window);
//     void parseEvents(sf::RenderWindow &window);
//     sf::Vector2f rayCast(sf::Vector2f from, sf::Vector2f dir, float max_length, Triangulation &cdt);


//     void shootGun(sf::Vector2f from, sf::Vector2f dir);

//     std::vector<RayInfo> findVisionField(sf::Vector2f from, float max_length, Triangulation &cdt);

//     std::vector<VisionCone> findVisionField2(sf::Vector2f from, float max_length, Triangulation &cdt);
//     std::vector<VisionCone> findVisionField2(sf::Vector2f from, float max_length, float angle_min, float angle_max);
//     std::vector<Edgef> findVisionFieldBasic(sf::Vector2f from, float max_length);

//     sf::VertexArray verticesFromRays(std::vector<Edgef> &rays);
//     sf::VertexArray verticesFromRays2(std::vector<RayInfo> &rays);
//     sf::VertexArray verticesFromRays3(sf::Vector2f from, std::vector<VisionCone> &rays);

//     void drawTriInd(sf::RenderWindow &window, Triangle &tri, TriInd i);

//     void smoothLights(const sf::RenderTexture &light_texture, sf::RenderTarget &scene_texture);

//     GameState state = GameState::RUNNING;

//     int meteor_spawner_time = 0;

//     sf::RenderWindow &m_window;

//     KeyBindings &key_binding;

//     Player m_player;
//     std::unique_ptr<VisionField> m_player_vision;

//     int m_num_rays = 10;
//     int m_num_interpol_rays = 0;
//     float m_vision_distance = 50;
//     float m_light_color[3] = {255, 255, 255};
//     bool m_lights_on = false;
//     sf::Clock m_clock;

//     std::shared_ptr<Triangulation> m_cdt;
//     std::shared_ptr<SearchGrid> m_grid;
//     std::unique_ptr<MapGrid> m_map;
//     std::unique_ptr<PathFinder> m_pathfinder;

//     sf::RenderTexture m_light_texture;
//     sf::RenderTexture m_light_cut;
//     sf::Shader m_full_pass;

//     sf::RenderTexture m_texture_pass[2];
//     sf::RenderTexture m_texture_combine;
//     sf::Shader m_light_smoother;
//     sf::Shader m_light_combiner;

//     sf::Font m_font;

//     bool m_point_visible = true;
//     PathFinder::PathAndPortals m_pap;

//     DynamicObjectPool<Zombie, 1000> m_zombies;
//     std::vector<std::vector<sf::Vector2f>> m_paths;
    
//     BoundingVolumeTree m_bvh;
//     std::vector<GunShot> m_shots;

//     GridNeighbourSearcher m_searcher;

//     float m_max_force = 1000.f;
//     float m_max_vel = 20.f;
//     float m_repulse_multiplier = 5.f;
//     float m_seek_multiplier = 500.f;
//     float m_hit_force = 5.f;
//     float m_friction = 0.5f;

//     void applyWallConstraint();

//     TriInd m_selected_tri_1 = -1;
//     TriInd m_selected_tri_2 = -1;

//     bool m_picking_edge = false;
//     bool m_changing_map = false;

//     friend UI;
// };

// #endif // BOIDS_GAME_H
#include "Enviroment.h"
#include "DrawLayer.h"
#include "MapGrid.h"

#include "GameWorld.h"
#include "Utils/RandomTools.h"

#include <Font.h>

EnviromentEffect::EnviromentEffect(TextureHolder &textures)
    : GameObject(&GameWorld::getWorld(), textures, ObjectType::VisualEffect)
{
}

FireEffect::FireEffect(ShaderHolder &shaders, TextureHolder &textures)
    : m_smoke(200), m_fire(200), EnviromentEffect(textures)
{
    m_smoke.setInitColor(m_smoke_color);
    m_smoke.setFinalColor(m_smoke_edge_color);
    m_smoke.setLifetime(2.f);
    m_smoke.setUpdater([this](Particle &p)
                       {
                                auto t_left = p.life_time - p.time;
                                p.acc = {0, 0};
                                p.vel += p.time *p.acc;
                                p.pos += p.vel * 0.016f;
                                p.scale += utils::Vector2f{0.5f};
                                p.angle += randf(0, 3.); });
    m_smoke.setEmitter([](utils::Vector2f spawn_pos)
                       {
                                Particle p;
                                p.pos = spawn_pos + utils::Vector2f{randf(-50,50), randf(0, 10.f)};
                                p.vel = {30+randf(-20, 20), randf(40, 50)};
                                p.scale = {10.2, 10.2};
                                p.life_time = 2.f;
                                return p; });
    m_smoke.setRepeat(true);

    m_fire.setLifetime(1.f);
    m_fire.setInitColor(m_fire_color);
    m_fire.setFinalColor(m_fire_edge_color);
    m_fire.setUpdater([this](Particle &p)
                      {
                                auto t_left = p.life_time - p.time;
                                p.acc = {0, 0};
                                p.vel += p.time *p.acc;
                                p.pos += p.vel * 0.016f;
                                p.angle += randf(0, 3.); });
    m_fire.setEmitter([](utils::Vector2f spawn_pos)
                      {
                                Particle p;
                                p.pos = spawn_pos + utils::Vector2f{randf(-5,5), randf(0, 1.f)};
                                p.vel = {10+randf(-30, 30), randf(40, 50)};
                                p.scale = {6.2, 6.2};
                                return p; });
    m_fire.setRepeat(true);
}

void FireEffect::update(float dt)
{
    m_transform.setPosition(50.f * utils::angle2dir(50));
}

void FireEffect::setEdgeColor(Color new_color)
{
    m_fire_edge_color = new_color;
}

void FireEffect::draw(LayersHolder &layers)
{
    auto &smoke_canvas = layers.getCanvas("Smoke");
    auto &fire_canvas = layers.getCanvas("Fire");
    auto &light_canvas = layers.getCanvas("Light");

    auto position = getPosition();
    m_smoke.setSpawnPos({position.x, position.y + 10}); //! smoke starts slightly above fire
    m_smoke.update(0.01f);
    m_smoke.draw(smoke_canvas);
    m_fire.setSpawnPos(position);
    m_fire.update(0.01f);
    m_fire.draw(fire_canvas);

    light_canvas.drawCricleBatched(m_pos, 100.f, {2, 1, 1, 1});
}

Water::Water(ShaderHolder &shaders, TextureHolder &textures)
    : m_water_verts(shaders.get("Water")), EnviromentEffect(textures)
{
}
void Water::setColor(Color new_color)
{
    m_water_color = new_color;
}

void Water::draw(LayersHolder &layers)
{
    assert(layers.hasLayer("Water"));
    auto &water_canvas = layers.getCanvas("Water");
    water_canvas.drawVertices(m_water_verts);
    m_target_size = water_canvas.getTargetSize();
}

void Water::readFromMap(cdt::Triangulation<cdt::Vector2i> &m_cdt, std::vector<int> &water_tri_inds)
{
    auto &triangles = m_cdt.m_triangles;

    auto normalize_tex = [this](cdt::Vector2f world_pos) -> utils::Vector2f
    {
        return {world_pos.x / m_target_size.x, world_pos.y / m_target_size.y};
    };

    auto n_verts_prev = m_water_verts.size();
    m_water_verts.resize(3 * water_tri_inds.size());
    for (int ind = 0; ind < water_tri_inds.size(); ++ind)
    {
        auto &tri = triangles.at(water_tri_inds.at(ind));
        m_water_verts[3 * ind + 0].pos = tri.verts[0];
        m_water_verts[3 * ind + 1].pos = tri.verts[1];
        m_water_verts[3 * ind + 2].pos = tri.verts[2];
        m_water_verts[3 * ind + 0].color = m_water_color;
        m_water_verts[3 * ind + 1].color = m_water_color;
        m_water_verts[3 * ind + 2].color = m_water_color;
        m_water_verts[3 * ind + 0].tex_coord = normalize_tex(tri.verts[0]);
        m_water_verts[3 * ind + 1].tex_coord = normalize_tex(tri.verts[1]);
        m_water_verts[3 * ind + 2].tex_coord = normalize_tex(tri.verts[2]);
    }
}

std::shared_ptr<Font> random_font = nullptr;

FloatingText::FloatingText(ShaderHolder &shaders, TextureHolder &textures)
    : EnviromentEffect(textures)
{
    setColor("EdgeColor", {0, 0, 0, 1});
    setColor("TextColor", {0, 0, 0, 1});

    if (!random_font)
    {
        random_font = std::make_shared<Font>("arial.ttf");
    }
    m_text.setFont(random_font);
    m_text.setText("TestFloat");
}

void FloatingText::draw(LayersHolder &layers)
{
    auto text_color = m_colors.at("TextColor");
    Color edge_color = m_colors.at("EdgeColor");
    glm::vec4 ec(edge_color.r, edge_color.g, edge_color.b, edge_color.a);
    auto &canvas = layers.getCanvas("Text");
    m_text.setColor(text_color);
    canvas.getShader("Text").getVariables().uniforms["u_edge_color"] = ec;
    canvas.drawText(m_text, "Text", GL_DYNAMIC_DRAW);
}

void FloatingText::update(float dt)
{
    m_time += dt;
    if (m_time > m_lifetime)
    {
        kill();
    }
    m_vel.y = 0.5f;
    m_text.setPosition(m_pos);
}
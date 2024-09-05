#include "Enviroment.h"
#include "DrawLayer.h"
#include "MapGrid.h"

#include "GameWorld.h"
#include "Utils/RandomTools.h"


EnviromentEffect::EnviromentEffect(TextureHolder& textures, ObjectType type)
: GameObject(&GameWorld::getWorld(), textures, type)
{

}

FireEffect::FireEffect(TextureHolder& textures)
    : m_smoke(200), m_fire(200), EnviromentEffect(textures, ObjectType::VisualEffect)
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

void FireEffect::setEdgeColor(Color new_color)
{
    m_fire_edge_color = new_color;
}

void FireEffect::draw(LayersHolder &layers)
{
    auto &smoke_canvas = layers.getCanvas("Smoke");
    auto &fire_canvas = layers.getCanvas("Fire");

    auto position = getPosition();
    m_smoke.setSpawnPos({position.x, position.y + 10}); //! smoke starts slightly above fire
    m_smoke.update(0.01f);
    m_smoke.draw(smoke_canvas);
    m_fire.setSpawnPos(position);
    m_fire.update(0.01f);
    m_fire.draw(fire_canvas);
}

Water::Water(ShaderHolder &shaders, LayersHolder &layers)
    : m_water_verts(shaders.get("Water"))
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
        m_water_verts[3 * ind].pos = tri.verts[0];
        m_water_verts[3 * ind + 1].pos = tri.verts[1];
        m_water_verts[3 * ind + 2].pos = tri.verts[2];
        m_water_verts[3 * ind].color = m_water_color;
        m_water_verts[3 * ind + 1].color = m_water_color;
        m_water_verts[3 * ind + 2].color = m_water_color;
        m_water_verts[3 * ind].tex_coord = normalize_tex(tri.verts[0]);
        m_water_verts[3 * ind + 1].tex_coord = normalize_tex(tri.verts[1]);
        m_water_verts[3 * ind + 2].tex_coord = normalize_tex(tri.verts[2]);
    }
}
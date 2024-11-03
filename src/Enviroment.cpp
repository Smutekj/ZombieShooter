#include "Enviroment.h"
#include "DrawLayer.h"
#include "MapGrid.h"

#include "GameWorld.h"
#include "LuaWrapper.h"
#include "Utils/RandomTools.h"

#include <LuaBridge/LuaBridge.h>
#include <LuaBridge/UnorderedMap.h>
// #include <LuaBridge/Vector.h>

#include <Font.h>

EnviromentEffect::EnviromentEffect(TextureHolder &textures)
    : GameObject(&GameWorld::getWorld(), textures, ObjectType::VisualEffect)
{
}
EnviromentEffect::EnviromentEffect(TextureHolder &textures, const std::string& effect_id)
    : GameObject(&GameWorld::getWorld(), textures, ObjectType::VisualEffect), m_effect_id(effect_id)
{
    // auto status = LuaWrapper::loadScript(script_name);
    // if (status != LuaScriptStatus::Broken)
    // {
        loadFromScript();
    // }
    m_collision_shape = std::make_unique<Polygon>(4);
}

static void setEmitter(Particles &particles, luabridge::LuaRef spawner, utils::Vector2f &vel)
{
    if (spawner.isFunction())
    {
        particles.setEmitter(
            [&vel, spawner](auto pos)
            {
                Particle p;
                try
                {
                    p = spawner(pos, vel);
                }
                catch (std::exception &e)
                {
                    std::cout << "ERROR IN Spawner" << e.what() << " !";
                }
                return p;
            });
    }
}

FireEffect::FireEffect(ShaderHolder &shaders, TextureHolder &textures)
    : m_smoke(200), m_fire(200), EnviromentEffect(textures)
{
    m_smoke.setInitColor(m_smoke_color);
    m_smoke.setFinalColor(m_smoke_edge_color);
    m_smoke.setLifetime(2.f);
    m_smoke.setUpdater([this](Particle &p, float dt = 0.016f)
                       {
                                auto t_left = p.life_time - p.time;
                                p.acc = {0, 0};
                                p.vel += p.time *p.acc;
                                p.pos += p.vel * dt;
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
    m_fire.setUpdater([this](Particle &p, float dt = 0.016f)
                      {
                                auto t_left = p.life_time - p.time;
                                p.acc = {0, 0};
                                p.vel += p.time *p.acc;
                                p.pos += p.vel * dt;
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
    m_collision_shape = std::make_unique<Polygon>(4);
}
void Water::setColor(Color new_color)
{
    m_water_color = new_color;
}

void Water::draw(LayersHolder &layers)
{
    // assert(layers.hasLayer("Water"));
    // auto &water_canvas = layers.getCanvas("Water");
    // water_canvas.drawVertices(m_water_verts);
    // m_target_size = water_canvas.getTargetSize();
}

void Water::readFromMap(cdt::Triangulation<cdt::Vector2i> &m_cdt, std::vector<int> &water_tri_inds)
{
    auto &triangles = m_cdt.m_triangles;

    auto normalize_tex = [this](cdt::Vector2f world_pos) -> utils::Vector2f
    {
        return {world_pos.x / m_target_size.x, world_pos.y / m_target_size.y};
    };

    auto max = [](const auto &r1, const auto &r2)
    {
        return utils::Vector2f{std::max(r1.x, r2.x), std::max(r1.y, r2.y)};
    };
    auto min = [](const auto &r1, const auto &r2)
    {
        return utils::Vector2f{std::min(r1.x, r2.x), std::min(r1.y, r2.y)};
    };

    utils::Vector2f max_r;
    utils::Vector2f min_r;

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

        max_r = max(asFloat(tri.verts[2]), max(tri.verts[0], tri.verts[1]));
        min_r = min(asFloat(tri.verts[2]), min(tri.verts[0], tri.verts[1]));
    }

    setPosition((max_r + min_r) / 2.f);
    setSize((max_r - min_r));
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
    m_text.setFont(random_font.get());
    m_text.setText("TestFloat");
}

void FloatingText::draw(LayersHolder &layers)
{
    auto text_color = m_colors.at("TextColor");
    Color edge_color = m_colors.at("EdgeColor");
    glm::vec4 ec(edge_color.r, edge_color.g, edge_color.b, edge_color.a);
    auto &canvas = layers.getCanvas("UI");
    m_text.setColor(text_color);
    canvas.getShader("Text").getVariables().uniforms["u_edge_color"] = ec;
    canvas.drawText(m_text, "Text", DrawType::Dynamic);
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

void EnviromentEffect::addParticles(const std::string &name, const std::string &layer, int max_particle_count)
{
    if (m_particles_holder.count(name) != 0)
    {
        m_particles_holder.insert({name, {layer, Particles{max_particle_count}}});
    }
}

Particles *EnviromentEffect::getParticlesP(const std::string &name)
{

    if (m_particles_holder.count(name) == 0)
    {
        return nullptr;
    }
    return &(m_particles_holder.find(name)->second.particles);
}

void EnviromentEffect::doScript(const std::string &script_name)
{
    auto lua = LuaWrapper::getSingleton();

    if (LuaWrapper::loadScript(script_name) != LuaScriptStatus::Ok)
    {
        return;
    }
}

void EnviromentEffect::update(float dt)
{

    if (m_lifetime > 0.f)
    {
        m_time += dt;
        if (m_time > m_lifetime)
        {
            kill();
        }
    }

    for (auto &[particles_name, draw_data] : m_particles_holder)
    {
        draw_data.particles.setSpawnPos(m_pos);
        draw_data.particles.update(dt);
    }

    auto script_status = LuaWrapper::loadScript(m_script_name);
    if (script_status == LuaScriptStatus::Ok || script_status == LuaScriptStatus::Broken)
    {
        return; //! do nothing since nothing changed or the file is broken
    }
    else
    {
        loadFromScript();
    }
}
void EnviromentEffect::draw(LayersHolder &layers)
{
    for (auto &[particles_name, draw_data] : m_particles_holder)
    {
        auto &layer_name = draw_data.layer_name;
        auto &canvas = layers.getCanvas(layer_name);
        draw_data.particles.draw(canvas);
    }

    //! draw script
    if (m_drawer)
    {
        m_drawer(layers);
    }
}

void EnviromentEffect::loadFromScript()
{
    auto lua = LuaWrapper::getSingleton();

    auto data_table_effects = getTable(lua->m_lua_state, "Effects");
    auto data_table = castToMap(data_table_effects);
    if(!data_table.at(m_effect_id).isTable())
    {
        return;
    }
    for (auto &[particles_name, p] : castToMap(data_table.at(m_effect_id)))
    {
        auto particles_table = data_table.at(particles_name);
        if (particles_table.isTable())
        {
            auto particle_data = particles_table.cast<std::unordered_map<std::string, luabridge::LuaRef>>();
            auto layer_name = particle_data.at("layer");
            auto shader_id = particle_data.at("shader");
            auto updater = particle_data.at("updater");
            auto emitter = particle_data.at("spawner");
            if (!(shader_id.isString() && layer_name.isString() && updater.isFunction() && emitter.isFunction()))
            {
                std::cout << "Types are wrong ParticleTable in script: " << m_effect_id << " in " << particles_name << "\n";
                continue;
            }

            m_particles_holder.insert({particles_name, {layer_name, Particles{200}}});
            auto &particles = m_particles_holder.at(particles_name).particles;
            particles.setShader(shader_id);
            particles.setUpdaterFull([this, updater](std::vector<Particle> &particles, int particle_count, float dt)
                                     {
                    try
                    {
                        updater(&particles, particle_count, dt);
                    }
                    catch (std::exception &e)
                    {
                        std::cout << "ERROR IN UpdaterFull: " << e.what() << " !";
                    } });
            particles.setEmitter([this, emitter](utils::Vector2f spawn_pos)
                                 {
                    Particle p;
                    try
                    {
                        auto &world = GameWorld::getWorld();
                        auto parent_id = world.m_scene.getNode(getId()).parent;
                        auto owner = world.get(parent_id);
                        auto vel = m_vel;
                        if (owner)
                        {
                            vel = owner->m_vel;
                        }
                        p = emitter(spawn_pos, vel);
                    }
                    catch (std::exception &e)
                    {
                        std::cout << "ERROR IN Spawner: " << e.what() << " !";
                    }
                    return p; });
        }
    }
    auto drawer_lua = data_table.at("Drawer");
    if (drawer_lua.isFunction())
    {

        m_drawer = [this, drawer_lua](LayersHolder &layers)
        {
            try
            {
                drawer_lua(this, layers);
            }
            catch (std::exception &e)
            {
                std::cout << "ERROR IN Drawer: " << e.what() << "\n";
            }
        };
    }
}

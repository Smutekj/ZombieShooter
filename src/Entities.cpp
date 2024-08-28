#include "Entities.h"
#include "Utils/RandomTools.h"

#include <Utils/Vector2.h>
#include <Particles.h>

#ifdef __cplusplus
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};
#endif //__cplusplus

PlayerEntity::PlayerEntity(GameWorld *world, TextureHolder &textures, ObjectType type)
    : GameObject(world, textures, type)
{
}

void PlayerEntity::update(float dt)
{
    m_pos += m_vel * dt;
}
void PlayerEntity::onCreation()
{
}
void PlayerEntity::onDestruction() {}
void PlayerEntity::draw(LayersHolder &layers)
{
    auto &target_canvas = layers.getCanvas("Unit");
    Sprite2 player_sprite(*m_textures.get("bomb"));
    player_sprite.setPosition(m_pos);
    player_sprite.setScale(m_size);
    player_sprite.setRotation(m_angle);
    target_canvas.drawSprite(player_sprite, "Instanced", GL_DYNAMIC_DRAW);
}
void PlayerEntity::onCollisionWith(GameObject &obj, CollisionData &c_data) {

};

Projectile::Projectile(GameWorld *world, TextureHolder &textures, ObjectType type)
    : GameObject(world, textures, type), m_bolt_particles(200)
{
    m_bolt_particles.setInitColor({0., 0.5, 20., 1.});
    m_bolt_particles.setFinalColor({1., 0.6, 2., 0.2});
    m_bolt_particles.setEmitter(
        [this](auto pos)
        {
            Particle p;
            p.pos = pos + m_size.x * utils::Vector2f{std::cos(randf(2 * M_PIf)), std::sin(randf(2 * M_PIf))} - m_vel;
            p.vel = {-m_vel.y, m_vel.x};
            p.vel *= (2. * randf() - 1.) / norm(m_vel) / 33.69f;
            return p;
        });
    m_bolt_particles.setUpdater(
        [](Particle &p)
        {
            p.scale = {2.f, 2.f};
            p.pos += p.vel;
        });
    m_bolt_particles.setLifetime(2.f);
    m_bolt_particles.setRepeat(true);
    setTarget(utils::Vector2f{0,0});
}

void Projectile::onCollisionWith(GameObject &obj, CollisionData &c_data) {

};
void Projectile::onCreation()
{
}

static float dir2angle(utils::Vector2f dir)
{
    return 180.f * std::atan2(dir.y, dir.x) / M_PIf;
}

void Projectile::onDestruction() {}
void Projectile::draw(LayersHolder &layers)
{

    int number = 0;

    // lua_State *L;
    // L = luaL_newstate();
    // lua_pushstring(L, "Anna");
    // lua_setglobal(L, "name");
    // luaL_openlibs(L);
    // if (luaL_dofile(L, "../scripts/hello.lua"))
    // {
    //     std::cout << "Final:" << lua_tostring(L, -1) << "\n";
    // }
    // if(lua_getglobal(L, "number"))
    // {

    // }
    // if (lua_isnil(L, -1))
    // {
    //     printf("is nil!\n");
    // }
    // else
    // {
    //     number = lua_tointeger(L, -1);
    //     printf("the number is %d\n", number);
    // }
    // lua_close(L);

    auto &canvas = layers.getCanvas("Unit");

    float angle = norm2(m_vel) > 0.f ? dir2angle(m_vel) : m_angle;
    setAngle(angle);

    m_bolt_particles.setSpawnPos(m_pos);
    m_bolt_particles.update(0.01f);
    m_bolt_particles.draw(canvas);
    canvas.drawCricleBatched(m_pos, m_angle, 1.1f * m_size.x, 0.9f * m_size.y, {0, 2.0, 10., 0.5});
}

void Projectile::update(float dt)
{
    std::visit(
        [this](auto &target)
        {
        using T = uncvref_t<decltype(target)>;
        if constexpr (std::is_same_v<T, GameObject *>)
        {
            if (target)
            {

                if (target->isDead())
                {
                    m_target = nullptr;
                }else{
                    m_last_target_pos = target->getPosition();
                }
            }
            else if constexpr (std::is_same_v<T, utils::Vector2f>)
            {
            }

        } },
        m_target);

    if(norm2(m_vel) > 0.0001)
    {

    }
    auto dr = m_last_target_pos - m_pos;
    auto dv = (dr / norm(dr) - m_vel / norm(m_vel));
    m_vel += dv * 0.5f;
    m_vel *= 50. / norm(m_vel);
    m_pos += m_vel * dt;
    setPosition(m_pos);

    m_time += dt;
    if (m_time > m_lifetime)
    {
        kill();
    }
}
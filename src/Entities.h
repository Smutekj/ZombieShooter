#pragma once

#include "GameObject.h"
#include <Particles.h>

#include "AILuaComponent.h"
#include "Shadows/VisibilityField.h"
#include "Map.h"

enum class Status
{
    Stunned,
    Slowed,
    Poisoned,
    Fire,
    Chilled,
    Frozen,
    Hasted
};

struct Statuses
{

    std::unordered_map<Status, float> m_status_timer;
};

enum class CombatState
{
    Casting,
    Attacking,
    Shooting,
    None, //
};

enum class DisabilityState
{
    Silenced,
    Rooted,
    Feared,
    Frozen,
    Stunned,
    Disarmed,
};

// struct DisabilitiesTable
// {
//     std::array
// };

enum class WeaponType
{
    Pistol,
    ShotGun,
    MachineGun,
    // RocketLauncher,
    // Grenade,
    // LaserGun,
};

struct WeaponData
{
    float m_shoot_cooldown_curr = 0.;
    float m_shoot_cooldown = 0.2;

    float m_reload_time_curr = 0.;
    float m_reload_time = 2.f;

    float m_heat_curr = 0.;
    float m_heat_max = 1.f;

    float m_range = 250.f;
    float m_spread = 0.5;

    float m_dmg = 2.f;
};

namespace Weapons
{
    static WeaponData s_pistol_data = {0, 0.35, 0, 1, 0., 1., 550., 0.5, 2.};
    static WeaponData s_shotgun_data = {0, 1.0, 0, 2, 0., 1., 150., 0.5, 6.};
    static WeaponData s_machinegun_data = {0, 0.1, 0, 2, 0., 1., 350., 0.5, 4.};
}

class Enemy;
class PlayerEntity : public GameObject
{

public:
    PlayerEntity() = default;
    PlayerEntity(TextureHolder &textures);
    virtual ~PlayerEntity() override {}

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    void setMaxSpeed(float max_speed);
    float getMaxSpeed() const;

    void shootWeapon();
    void switchWeaponNext();
    void switchWeaponPrev();

private:
    void shootBullet(utils::Vector2f from, float angle_dir, float vel);
    // void shootPistol();
    // void shootShotGun();

    void doScript();

    static std::unordered_map<WeaponType, WeaponData> m_weapons;

public:
    float m_vision_radius = 300.f;
    float m_max_speed = 300.f;
    float m_cast_time = -1.f;

    float m_aim_angle = 0.;


    CombatState m_combat_state = CombatState::None;
    Enemy *target_enemy = nullptr;

    VisionField m_vision;
    bool m_holding_trigger = false; 

    int m_selected_ability = 3;

private:
    std::unordered_set<DisabilityState> m_disabilities;

    VertexArray m_vision_verts;

    WeaponType m_selected_weapon = WeaponType::Pistol;

};  

class Event : public GameObject
{

public:
    Event(TextureHolder &textures, std::string event_script_name);
    virtual ~Event() override {}

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

public:
    std::string m_script_name;
};

using ProjectileTarget = std::variant<GameObject *, utils::Vector2f>;
using ProjectileCaster = std::variant<GameObject *, PlayerEntity *>;

namespace Collisions
{
    class CollisionSystem;
}

namespace pathfinding
{
    class PathFinder;
}

    enum class MotionState
    {
        Attacking = 0,
        Casting = 1,
        Chasing =2,
        Running = 3,
        Walking = 4,
        Standing = 5,
    };


class Enemy : public GameObject
{

public:
    Enemy() = default;
    Enemy(pathfinding::PathFinder &pf, TextureHolder &textures, Collisions::CollisionSystem &collider, PlayerEntity *player);
    virtual ~Enemy() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

public:
    int getState() const
    {
        return static_cast<int>(m_state);
    }
    int getMotionState() const
    {
        return static_cast<int>(m_motion_state);
    }
    void setState(int state)
    {
        if (state >= 3)
        {
            return;
        }
        m_state = static_cast<AIState>(state);
    }
    void setMotionState(int state)
    {
        if (state >= 6)
        {
            return;
        }
        m_motion_state = static_cast<MotionState>(state);
    }

    const std::string &getScript() const
    {
        return m_script_name;
    }

    void setScript(const std::string script_name)
    {
        m_script_name = script_name;
    }

private:
    void avoidMeteors();
    void boidSteering();
    void doScript();

public:

    float max_vel = 100.f;
    float max_acc = 550.f;
    float max_impulse_vel = 40.f;

    float m_health = 10;
    float m_max_health = 10;
    utils::Vector2f m_impulse = {0, 0};

    utils::Vector2f m_next_path = {-1, -1};
    utils::Vector2f m_next_next_path = {-1, -1};

    AIState m_state = AIState::Patroling;
    MotionState m_motion_state = MotionState::Walking;

    Color m_color = Color(20, 0, 0, 1);

public:
    static std::unordered_map<Multiplier, float> m_force_multipliers;
    static std::unordered_map<Multiplier, float> m_force_ranges;

private:
    std::function<void()> m_ai_updater;
    std::function<bool(cdt::TriInd, cdt::TriInd)> m_path_rule = [](cdt::TriInd i1, cdt::TriInd i2)
    { return true; };

    float m_boid_radius = 30.f;
    utils::Vector2f m_acc = {0, 0};

    Collisions::CollisionSystem *m_collision_system = nullptr;
    pathfinding::PathFinder *m_pf = nullptr;
    PlayerEntity *m_player = nullptr;

    bool m_is_avoiding = false;

    int m_pathfinding_timer = 0;
    int m_pathfinding_cd = 120;
    std::string m_script_name = "basicai.lua";

    SurfaceTable m_surfaces;
    // std::array<bool, static_cast<int>(SurfaceType::Count)> m_surface_is_walkable;
};

template <class T>
using uncvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
template <typename T>
struct fail : std::false_type
{
};

class Bullet : public GameObject
{

public:
    using ColorTable = std::map<std::string, std::map<std::string, float>>;

public:
    Bullet() = default;
    Bullet(TextureHolder &textures);
    virtual ~Bullet() override {}

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    // std::function<utils::Vector2f(utils::Vector2f, float)> m_intergrator = [](utils::Vector2f pos, float dt){;};
    float m_max_speed = 1500.f;
    float m_impulse = m_max_speed;
    float m_dmg = 1.;
};

class Projectile : public GameObject
{

public:
    using ColorTable = std::map<std::string, std::map<std::string, float>>;

public:
    Projectile() = default;
    Projectile(GameWorld *world, TextureHolder &textures, const std::string &script_name = "firebolt.lua");
    virtual ~Projectile() override {}

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    void setScriptName(const std::string &new_script_name);

    const std::string &getScriptName() const;

    void setMaxVel(float new_vel);

    float getMaxVel() const;

    void setAcc(float new_acc);

    float getAcc() const;

    void setTarget(ProjectileTarget target);

public:
    int m_owner_entity_id = -1;
    bool m_homing = false;

private:
    void readDataFromScript();

protected:
    // ProjectileTarget m_target = nullptr;
    utils::Vector2f m_last_target_pos = {0, 0};
    float m_max_vel = 50.f;
    float m_acc = 20.f;

    Particles m_bolt_particles;
    std::unordered_map<std::string, std::shared_ptr<Particles>> m_particles;
    float m_time = 0.f;
    float m_lifetime = 10.f;

    std::string m_shader_name = "fireBolt";
    std::string m_script_name = "firebolt";
    std::string m_bolt_canvas_name = "Wall";
    std::string m_tail_canvas_name = "Wall";
};

struct Timer
{
    float m_lifetime = 0.f;
    float m_time = 0.f;
};

class OrbitingShield : public GameObject
{
public:
    OrbitingShield() = default;
    OrbitingShield(GameWorld *world, TextureHolder &textures);
    virtual ~OrbitingShield() override {}

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    void readDataFromScript();

public:
    std::string m_script_name = "basicshield.lua";
    std::string m_shader_name = "basicshield";

private:
    float m_orbit_speed = 2.f;
    float m_time = 0.f;
    float m_lifetime = 500000.f;

    std::unique_ptr<Particles> m_particles;
};
class Shield : public GameObject
{
public:
    Shield() = default;
    Shield(TextureHolder &textures);
    virtual ~Shield() override {}

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &layers) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    float m_orbit_speed = 2.f;
    float m_time = 0.f;
    float m_lifetime = 500000.f;

    std::unique_ptr<Particles> m_particles;
};

struct ProjectileData
{
    GameObject *m_caster = nullptr;
    GameObject *m_target = nullptr;
};

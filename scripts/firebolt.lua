




InitColor = Color(1000., 0.5, 0.1, 0.8);
FinalColor = Color(2000., 10., 0., 0.2);
SpawnPeriod = 1;

TailShader = ""
BoltShader = "fireBolt"

TailCanvas = "Wall";
BoltCanvas = "Fire";

ParticleColors = {
    Init =  {r = 40.,g = 0.2,b = 0.,a = 1.},
    Final =  {r = 0.,g = 1.,b = 0.,a = 1.}
}

BoltColor = {r = 20.,g = 0.2,b = 0.,a = 1.}
BoltSpeed = 250.;

function Updater(particle)
    particle.pos = particle.pos + particle.vel;
    p.scale.x = p.scale.x - 0.2;
    p.scale.y = p.scale.y -0.2;
    p.angle = p.angle + 10;
    return particle;
end

function randVec()
    rand_angle = math.random()*math.pi;
    return Vec(math.cos(rand_angle), math.sin(rand_angle));
end

function randVec(size)
    rand_angle = math.random()*math.pi;
    return Vec(size*math.cos(rand_angle), size*math.sin(rand_angle));
end

-- function Spawner(spawn_pos, spawn_vel)
--     local spawn_mult = -100.00;
--     local speed = 20.;

--     local p = Particle(spawn_pos.x, spawn_pos.y);
--     local rand_angle = math.random()*math.pi;
--     local dr = randVec(10.);
--     p.pos = spawn_pos + dr;
    
--     local sign = math.random(-1,1)
--     local n_dir = Vec(sign*spawn_vel.y, -sign*spawn_vel.x);
--     p.vel.x = speed*n_dir.x + spawn_vel.x*spawn_mult;
--     p.vel.y = speed*n_dir.y + spawn_vel.y*spawn_mult;
    
--     p.life_time = 50.;
    
--     p.angle = math.random()*90.;
--     return p;
-- end

function Spawner(spawn_pos, spawn_vel)

    local spawn_mult = 0.005;
    local speed = 0.05;

    local p = Particle(spawn_pos.x, spawn_pos.y);

    local rand_angle = math.random()*math.pi;
    local dr = randVec(10.);
    p.pos = spawn_pos + dr;
    
    local sign = math.random(-1,1)
    local n_dir = Vec(sign*spawn_vel.y, -sign*spawn_vel.x);
    p.vel.x = speed*n_dir.x + spawn_vel.x*spawn_mult;
    p.vel.y = speed*n_dir.y + spawn_vel.y*spawn_mult;

    p.life_time = 1.;
    p.scale.x = p.scale.x + 5.;
    p.scale.y = p.scale.y + 5.;

    return p;
end
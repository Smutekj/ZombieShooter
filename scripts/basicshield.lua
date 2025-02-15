


InitColor = Color(100., 0.5, 0., 1.);
FinalColor = Color(10., 1., 10., 0.2);
SpawnPeriod = 1;

TailShader = "lightning"
BoltShader = "basicshield"

BoltColor = {r = 0.2,g = 2.,b = 69.,a = 1.}

local function randVec()
    rand_angle = math.random()*math.pi;
    return Vec(math.cos(rand_angle), math.sin(rand_angle));
end

local function randVec(size)
    rand_angle = math.random()*math.pi;
    return Vec(size*math.cos(rand_angle), size*math.sin(rand_angle));
end

local function angle(dir)
    return math.deg(math.atan(dir.y, dir.x));
end

function Updater(particle)
    particle.vel.x = (particle.vel.x)*0.85;
    particle.vel.y = (particle.vel.y)*0.85;
    particle.pos = particle.pos + particle.vel;
    return particle;
end



function math.norm(v)
    return math.sqrt(v.x*v.x + v.y*v.y);
end

GameTimeStep = 0.1; -- dt used in world.update(dt);

function Spawner(spawn_pos, spawn_vel)

    local size = 39.;
    local speed = 0.01;

    local rand_angle = math.random()*2.*math.pi;
    local dr = Vec(math.sin(rand_angle)*size, math.cos(rand_angle)*1/0.7*size);
    local p = Particle(spawn_pos.x, spawn_pos.y);
    
    p.pos.x = spawn_pos.x + dr.x;
    p.pos.y = spawn_pos.y + dr.y;
    local norm = math.sqrt(dr.x*dr.x + dr.y*dr.y);
    local norm_spawn_vel = math.norm(spawn_vel);
    p.vel.x = dr.x/norm*speed + spawn_vel.x*GameTimeStep;
    p.vel.y = dr.y/norm*speed + spawn_vel.y*GameTimeStep;

    p.angle = angle(dr);
    if (norm_spawn_vel > 0) then    
        p.life_time = math.min(2./norm_spawn_vel, 0.5);
        print(p.life_time);
    else
        p.life_time = 0.5;
    end
    print(p.life_time);
    p.scale.x = 10.;
    p.scale.y = 0.5;
    return p;
end
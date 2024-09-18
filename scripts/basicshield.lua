


InitColor = Color(0., 100., 0., 1.);
FinalColor = Color(10., 1., 10., 0.2);

TailShader = "fireBolt"
BoltShader = "basicshield"



BoltColor = {r = 0.2,g = 2.,b = 69.,a = 1.}

function randVec()
    rand_angle = math.random()*math.pi;
    return Vec(math.cos(rand_angle), math.sin(rand_angle));
end

function randVec(size)
    rand_angle = math.random()*math.pi;
    return Vec(size*math.cos(rand_angle), size*math.sin(rand_angle));
end


function Updater(particle)

    -- print("prev: ", particle.pos.x, particle.pos.y);
    particle.pos = particle.pos + particle.vel;
    return particle;
    -- print("after: ", particle.pos.x, particle.pos.y);
end

local function angle(dir)
    return math.deg(math.atan(dir.y, dir.x));
end

function Spawner(spawn_pos, spawn_vel)

    local size = 39.;
    local speed = 0.076;

    local rand_angle = math.random()*2.*math.pi;
    local dr = Vec(math.sin(rand_angle)*size, math.cos(rand_angle)*1/0.7*size);
    local p = Particle(spawn_pos.x, spawn_pos.y);
    
    p.pos.x = spawn_pos.x + dr.x;
    p.pos.y = spawn_pos.y + dr.y;
    print(spawn_vel.x, spawn_vel.y);
    local norm = math.sqrt(dr.x*dr.x + dr.y*dr.y);
    p.vel.x = dr.x/norm*speed - spawn_vel.x;
    p.vel.y = dr.y/norm*speed - spawn_vel.y;
    p.angle = angle(p.vel)
    p.scale.x = 10.;
    p.scale.y = 0.5;
    return p;
end
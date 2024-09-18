

InitColor = Color(100., 1., 0., 1.);
FinalColor = Color(0., 1., 100., 1.);

TailShader = ""
BoltShader = "fireBolt"

ParticleColors = {
    Init =  {r = 40.,g = 0.2,b = 0.,a = 1.},
    Final =  {r = 0.,g = 1.,b = 0.,a = 1.}
}

BoltColor = {r = 20.,g = 0.2,b = 0.,a = 1.}



function Updater(particle)
    particle.pos = particle.pos + particle.vel;
end

function randVec()
    rand_angle = math.random()*math.pi;
    return Vec(math.cos(rand_angle), math.sin(rand_angle));
end

function randVec(size)
    rand_angle = math.random()*math.pi;
    return Vec(size*math.cos(rand_angle), size*math.sin(rand_angle));
end

function Spawner(spawn_pos, dir)

    spawn_pos.x = spawn_pos.x;
    p = Particle(spawn_pos.x, spawn_pos.y);

    rand_angle = math.random()*math.pi;
    dr = randVec(10.);

    p.pos = spawn_pos + dr;
    
    sign = math.random(-1,1)
    n_dir = Vec(sign*dir.y, -sign*dir.x);
    p.vel.x = 0.05*n_dir.x;
    p.vel.y = 0.05*n_dir.y;
    return p;
end
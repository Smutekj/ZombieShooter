
local function randVec()
    local rand_angle = math.random() * 2. * math.pi;
    return Vec(math.cos(rand_angle), math.sin(rand_angle));
end

local function randVec(size)
    local rand_angle = math.random() * 2. * math.pi;
    local result = Vec(0, 0);
    result.x = size * math.cos(rand_angle);
    result.y = size * math.sin(rand_angle);
    return result;
end

Particles1 =
{
    name = "DefaultParticles",
    shader = "Default",
    layer = "Wall",
    lifetime = 2.,
    updater = function(particles, n_particles, dt)
        -- print(table.getn(particles))
        for i = 0, n_particles - 1, 1 do
            local p = particles:at(i);
            -- print(p.time)
            p.pos.x = p.pos.x + p.vel.x * dt;
            p.pos.y = p.pos.y + p.vel.y * dt;
            p.angle = p.angle + (2 * (i % 2) - 1) * 5;
            p.time = p.time + dt; 
            p.scale = Vec(p.scale.x + 0.1, p.scale.y + 0.1);
            p.color.a = p.color.a*0.95;
        end
    end,
    spawner = function(spawn_pos, spawn_vel)
        local spawn_mult = 0.005;
        local speed = 20.;
    
        local p = Particle(spawn_pos.x, spawn_pos.y);
        local rand_angle = math.random() * math.pi;
        local dr = randVec(100.);
        p.pos = spawn_pos + dr;
    
        local sign = math.random(-1, 1)
        local n_dir = Vec(sign * spawn_vel.y, -sign * spawn_vel.x);
        n_dir.x = sign * spawn_vel.y;
        n_dir.y = -sign * spawn_vel.x;
        p.vel.x = 5. * (math.random() - 0.5);
        p.vel.y = 20.;
    
        p.life_time = 1.5;
        p.scale = Vec(1., 1.);
    
        p.color = Color(0, 1000, 0, 1);
        return p;
    end,
};

Particles2 =
{
    updater = function(particles, n_particles, dt)
        -- print(table.getn(particles))
        for i = 0, n_particles - 1, 1 do
            local p = particles:at(i);
            -- print(p.time)
            p.pos.x = p.pos.x + p.vel.x * dt;
            p.pos.y = p.pos.y + p.vel.y * dt;
            p.angle = p.angle + (2 * (i % 2) - 1) * 5;
            p.time = p.time + dt;
        end
    end,
    spawner = function(spawn_pos, spawn_vel)
        local p = Particle(spawn_pos.x, spawn_pos.y);
        local dr = randVec(10.);
        p.pos = spawn_pos + dr;
        local sign = math.random(-1, 1)
        p.vel.x = 5. * (math.random() - 0.5);
        p.vel.y = 2.;

        p.life_time = 1.5;
        p.scale = Vec(5., 5.);

        p.color = Color(0, 100, 1, 0);
        -- print(p.vel.x, p.vel.y, n_dir.y)
        return p;
    end,
    shader = "Default",
    layer = "Fire",
    name = "BigPenisParticles",
    lifetime = 2.,
};

Effect = {}
-- Effect["Particles1"] = Particles1;
-- Effect["Particles2"] = Particles2;

local function drawTheThing(position, layers, radius)
    local thickness = radius / 10.;
    local prev_pos = Vec(position.x + radius, position.y);

    local texture = getTexture("coin");
    local sprite = Sprite();
    sprite.setTexture(sprite, 0, texture);
    sprite.pos = position;
    sprite.scale = Vec(200., 200.);
    layers.drawSprite(layers, "Wall", sprite, "consecration");

end

Effect.Drawer = function(effect, layers)
    local c = Color(1, 0, 0, 1);
    local eye_color = Color(10, 0, 0, 1); 
    drawTheThing(effect.pos, layers, 10.);
end

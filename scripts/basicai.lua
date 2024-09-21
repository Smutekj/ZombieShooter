PATROLING = 0
CHASING = 1
ATTACKING = 2

local attack_range = 200
local vision_range = 500

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

local function dist(x1, y1, x2, y2)
    local dx = x2 - x1
    local dy = y2 - y1
    return math.sqrt(dx * dx + dy * dy)
end
-- local function dist(r1, r2)
--     local dx = r2.x - r1.x
--     local dy = r2.y - r1.y
--     return math.sqrt(dx * dx + dy * dy)
-- end

local function inRange(enemy, target, range)
    return dist(target.x, target.y, enemy.x, enemy.y) < range;
end



local function onAttack(enemy)
    if (enemy.target == nil) then
        return
    end
    local target = enemy.target;
    if Timers[enemy.id] == nil then
        Timers[enemy.id] = 0;
    end
    local timer = Timers[enemy.id];
    timer = timer + 1;
    if timer > 100 then
        timer = 0;
        if inRange(enemy, target, attack_range) then
            local p = createProjectile("Bullet", "P");
            p.x = enemy.x;
            p.y = enemy.y;
            p.owner = enemy.id;
            p.vel.x = target.x - enemy.x;
            p.vel.y = target.y - enemy.y;
            p.target = target
            enemy.vel.x = 0;
            enemy.vel.y = 0;
            p.setScript(p, "firebolt.lua");
        else
            enemy.state = CHASING;
        end
    end
    Timers[enemy.id] = timer;
end


local function onChase(enemy)
    if enemy.target == nil then
        return
    end
    local target = enemy.target;


    if ChaseTimers[enemy.id] == nil then
        ChaseTimers[enemy.id] = 0;
    end
    local timer = ChaseTimers[enemy.id];
    timer = timer + 1;
    if timer > 100 then
        timer = 0;
        if not inRange(enemy, target, vision_range) then
            enemy.state = PATROLING;
            enemy.target = nil
            return;
        elseif inRange(enemy, target, attack_range) then
            enemy.state = ATTACKING;
            timer = 0;
            enemy.vel.x = 0;
            enemy.vel.y = 0;
        end
    end
    ChaseTimers[enemy.id] = timer;
    -- print(ChaseTimers[enemy.id])
end

local function isInMap(r)
    return r.x > 0 and r.y > 0 and r.x < 2000 and r.y < 2000;
end

local function generateNewTargetPos(enemy)
    local rand_dr = randVec(500.);
    enemy.target_pos.x = enemy.target_pos.x + rand_dr.x;
    enemy.target_pos.y = enemy.target_pos.y + rand_dr.y;
end
local function onPatrol(enemy)
    if not (enemy.target == nil) then
        if inRange(enemy, enemy.target, vision_range) then
            enemy.state = CHASING
            return;
        end
    end

    -- print("enemy pos: " .. enemy.target_pos.x, enemy.target_pos.y);
    if dist(enemy.target_pos.x, enemy.target_pos.y, enemy.x, enemy.y) < 400. then
        local n_attempts = 0;
        local new_pos = enemy.target_pos;
        repeat
            local rand_dr = randVec(500.);
            -- print(rand_dr.x, rand_dr.y)
            new_pos = enemy.target_pos + rand_dr;
            n_attempts = n_attempts + 1;
        until n_attempts < 200 or isInMap(new_pos)

        if isInMap(new_pos) then
            enemy.target_pos = new_pos;
            print(enemy.target_pos.y)
            print("HI")
        end
    end

    player = getObject("Player");
    if inRange(enemy, player, vision_range) then
        enemy.state = CHASING
        enemy.target = player;
        return;
    end
end

local behaviour_table = { attack = onAttack }

local function isempty(s)
    return s == nil
end

local function updateFSM(enemy)
    if enemy.state == PATROLING then
        onPatrol(enemy);
        return;
    elseif enemy.state == ATTACKING then
        onAttack(enemy);
        return;
    elseif enemy.state == CHASING then
        onChase(enemy);
        return;
    else
        print("Invalid State");
    end
end

function updateAI(enemy) --object is the c++ passed function
    if (isempty(Time)) then
        Time = 0;
    end
    Time = Time + 1;
    -- enemy.state = PATROLING;
    -- enemy.target = nil;
    -- enemy.target = getObject("Player");
    updateFSM(enemy);
    -- print(Time)
end

local function drawAgent(enemy, layers, radius, color, eye_color)
    local thickness = radius / 10.;
    local position = enemy.pos;
    local prev_pos = Vec(position.x + radius, position.y);
    -- for i = 0, 51 do
    --     local x = position.x + radius * math.cos(i / 50. * 2. * math.pi);
    --     local y = position.y + radius * math.sin(i / 50. * 2. * math.pi);
    --     layers.drawLine(layers, "Unit", prev_pos, Vec(x, y), thickness, color);
    --     prev_pos = Vec(x, y);
    -- end
    -- local left_eye_pos1 = position + Vec(-radius * 0.2, radius * 0.4);
    -- local left_eye_pos2 = position + Vec(-radius * 0.6, radius * 0.5);
    -- local right_eye_pos1 = position + Vec(radius * 0.2, radius * 0.4);
    -- local right_eye_pos2 = position + Vec(radius * 0.6, radius * 0.5);
    -- layers.drawLine(layers, "Fire", left_eye_pos1, left_eye_pos2, thickness / 2., eye_color);
    -- layers.drawLine(layers, "Fire", right_eye_pos1, right_eye_pos2, thickness / 2., eye_color);

    local texture = getTexture("coin");
    local sprite = Sprite();
    sprite.setTexture(sprite, 0, texture);
    sprite.pos = Vec(enemy.x, enemy.y);
    sprite.scale = Vec(15.,15.);
    -- if norm(enemy.vel) > 0 then
    --     sprite.angle = math.deg(math.atan(enemy.vel.y, enemy.vel.x));
    -- else
        sprite.angle = math.rad(enemy.angle);
    -- end

    layers.drawSprite(layers, "Wall", sprite, "basicagent");
    layers.drawLine(layers, "Unit", enemy.pos, enemy.pos + enemy.vel, thickness / 2., eye_color);
end

function Draw(enemy, layers) --object is the c++ passed function
    local c = Color(1, 0, 0, 1);
    local eye_color = Color(10, 0, 0, 1);
    drawAgent(enemy, layers, 10., c, eye_color);
end

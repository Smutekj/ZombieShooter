PATROLING = 0
CHASING = 1
ATTACKING = 2

local attack_range = 100
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

local function distOld(x1, y1, x2, y2)
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
    return dist(target, enemy.pos) < range;
end



local function onAttack(enemy)
    if (enemy.target == nil) then
        return
    end
    local target = enemy.target;
    -- if Timers[enemy.id] == nil then
    --     Timers[enemy.id] = 0;
    -- end
    -- local timer = Timers[enemy.id];
    -- timer = timer + 1;
    -- if timer > 100 then
    --     timer = 0;
    if inRange(enemy, target, attack_range) then
        Entity2Abilities[enemy.id][FireBoltAbility.spell_id].onUse(enemy, target);
    else
        enemy.state = CHASING;
    end
    -- end
    -- Timers[enemy.id] = timer;
end


local function onChase(enemy)
    if enemy.target == nil then -- don't chase when no target exists
        enemy.state = PATROLING;
        return
    end
    local target = enemy.target;

    if ChaseTimers[enemy.id] > 5 then -- each 5 seconds check if target is in range
        ChaseTimers[enemy.id] = 0;
        if not inRange(enemy, target, vision_range) then -- if out of vision we stop chasing
            enemy.state = PATROLING;
            enemy.target = nil
            return;
        elseif inRange(enemy, target, attack_range) then
            enemy.state = ATTACKING;
            enemy.vel.x = 0;
            enemy.vel.y = 0;
        end
    end
end

local function isInMap(r)
    return r.x > 0 and r.y > 0 and r.x < 500 and r.y < 500;
end

local function onPatrol(enemy)
    if not (enemy.target == nil) then
        if inRange(enemy, enemy.target, vision_range) then
            enemy.state = CHASING
            return;
        end
    end
    -- print("HI")
    -- print("enemy pos: " .. enemy.target_pos.x, enemy.target_pos.y);
    if dist(enemy.target_pos, enemy.pos) < 50. then
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
            -- print(enemy.target_pos.y)
        end
    end

    local player = getObject("Player");
    -- print(dist(player.pos, enemy.pos))
    if inRange(enemy, player, vision_range) then
        print("Penis2")
        enemy.state = CHASING
        enemy.target = player;
        return;
    end
end

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

AIStates = { "PATROLING", "CHASING", "ATTACKING" };

function updateAI(enemy) --object is the c++ passed function
    if (isempty(Time)) then
        Time = 0;
    end
    -- print("ID: " .. enemy.id .. " pos: " .. enemy.pos.x .. " " .. enemy.pos.y)
    Time = Time + 1;
    print(enemy.id)
    enemy.max_acc = 1000.;
    -- print("Enemy with ID: " .. enemy.id .. " is: " .. AIStates[enemy.state + 1])
    updateFSM(enemy);

    -- local e = getObject("E4");
    -- print(e.pos.x, e.pos.y)
    -- local e = getObject("E48");
    -- print(e.target_pos.x, e.target_pos.y)
    -- print(Time)
end


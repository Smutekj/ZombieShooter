PATROLING = 0
CHASING = 1
ATTACKING = 2

local attack_range = 50
local vision_range = 100

local function dist(x1, y1, x2, y2)
    local dx = x2 - x1
    local dy = y2 - y1
    return math.sqrt(dx * dx + dy * dy)
end

local function inRange(enemy, target, range)
    return dist(target.x, target.y, enemy.x, enemy.y) < range;
end



local function onAttack(enemy)
    local target = enemy.target;
    if Timers[enemy.id] == nil then
        Timers[enemy.id] = 0 ;
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
            p.setScript(p, "frostbolt.lua");
        else
            enemy.state = CHASING;
        end
    end
    Timers[enemy.id] = timer;
end


local function onChase(enemy)
    local target = enemy.target;
    if ChaseTimers[enemy.id] == nil then
        ChaseTimers[enemy.id] = 0 ;
    end
    local timer = ChaseTimers[enemy.id];
    timer = timer + 1;
    if timer > 100 then 
        timer = 0;
        if not inRange(enemy, target, vision_range) then
            enemy.state = PATROLING;
            return;
        elseif inRange(enemy, target, attack_range) then
            enemy.state = ATTACKING;
            timer = 0;
            enemy.vel.x = 0;
            enemy.vel.y = 0;
        end
    end
    ChaseTimers[enemy.id] = timer;
    print(timer)
    -- print(ChaseTimers[enemy.id])
end

local function onPatrol(enemy)
    if inRange(enemy, enemy.target, vision_range) then
        enemy.state = CHASING
    end
end

local behaviour_table = { attack = onAttack }

local function isempty(s)
    return s == nil
end

local function updateFSM(enemy)
    print(enemy.state);
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
    print((getName(enemy.id)))
    enemy.target = getObject("Player");
    updateFSM(enemy);
    -- print(Time)
end

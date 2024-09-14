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
    print(AttackTimer)
    if AttackTimer == nil then
        AttackTimer = 100
    end
    if AttackTimer == 0 then
        if inRange(enemy, target, attack_range) then
            local p = createObject("Bullet", "P");
            p.x = enemy.x;
            p.y = enemy.y;
            p.vel.x = target.x - enemy.x;
            p.vel.y = target.y - enemy.y;
            p.target = target
            AttackTimer = 200;
        else
            enemy.state = CHASING;
        end
    end
    AttackTimer = AttackTimer - 1;
end
local function onChase(enemy)
    local target = enemy.target;
    if (dist(target.x, target.y, enemy.x, enemy.y) < attack_range) then
        AttackTimer = 100;
        enemy.state = ATTACKING;
    end
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
    updateFSM(enemy);
end

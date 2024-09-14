PATROLING = 0
CHASING = 1
ATTACKING = 2

local attack_range = 5

local function onAttack(posx, posy, dist)
    if(dist < attack_range) then
        createObject("Bullet", "P")
        setPosition("P", posx, posy)
    end
    return 1
end

local behaviour_table = {attack = onAttack}

local function dist(x1, y1, x2, y2)
    local dx = x2 - x1
    local dy = y2 - y1
    return math.sqrt(dx*dx + dy*dy)
end



function Update(time) --object is the c++ passed function
    -- print(time)
    if time > 2 then
        -- createObject("Enemy", "E")
        -- setPosition("E", 200, 200)
        time = 0.
    end
 end
 


ENEMY        = 0
BULLET       = 1
PLAYER       = 2
WALL         = 3
ORBITER      = 4
VISUALEFFECT = 5


function dot(v1, v2)
    return v1.x * v2.x + v1.y * v2.y;
end

function norm2(v1)
    return dot(v1, v1);
end

function norm(v1)
    return math.sqrt(norm2(v1));
end

function normalize(v1)
    local n = norm(v1);
    v1.x = v1.x / n;
    v1.y = v1.y / n;
end

Repulsion_Factor = 2000.5; 
Exponent = 2;

function RepulsePairForce(obj1, obj2)
    local dr = Vec(obj2.x - obj1.x, obj2.y - obj1.y);
    local dist = norm(dr);
    normalize(dr);


    obj1.vel.x = obj1.vel.x - Repulsion_Factor * dr.x * dist ^ (-Exponent);
    obj1.vel.y = obj1.vel.y - Repulsion_Factor * dr.y * dist ^ (-Exponent);
    obj2.vel.x = obj2.vel.x + Repulsion_Factor * dr.x * dist ^ (-Exponent);
    obj2.vel.y = obj2.vel.y + Repulsion_Factor * dr.y * dist ^ (-Exponent);

    Truncate(obj1.vel, 100.9)
    Truncate(obj2.vel, 100.9)
end

function Truncate(v, max_size)
    local size = norm(v);
    if norm(v) > max_size then
        v.x = v.x/size * max_size;
        v.y = v.y/size * max_size;
    end
end

function RepulsePairForce2(obj1, obj2)
    local dr = Vec(obj2.x - obj1.x, obj2.y - obj1.y);
    local dist = norm(dr);
    normalize(dr);

    local prev_norm_vel1 = norm(obj1.vel);
    local prev_norm_vel2 = norm(obj2.vel);

    obj1.vel.x = obj1.vel.x - Repulsion_Factor * dr.x * dist ^ (-Exponent);
    obj1.vel.y = obj1.vel.y - Repulsion_Factor * dr.y * dist ^ (-Exponent);
    obj2.vel.x = obj2.vel.x + Repulsion_Factor * dr.x * dist ^ (-Exponent);
    obj2.vel.y = obj2.vel.y + Repulsion_Factor * dr.y * dist ^ (-Exponent);

    Truncate(obj1.vel, 50.6)
    Truncate(obj2.vel, 50.6)
end


function EnemyPlayerCollision(obj1, obj2)
--     local dr = Vec(obj2.x - obj1.x, obj2.y - obj1.y);
--     normalize(dr);
--     -- local dist = norm(dr);
-- -- 
--     RepulsePairForce2(obj1, obj2);
--     local v1 = obj1.vel;
--     local v1_in_dr = dot(v1, dr);
--     local v2 = obj2.vel;
--     local v2_in_dr = -dot(v2, dr);
    
    -- if dist > 10. then
    --     v1_in_dr = 0.;
    --     v2_in_dr = 0.;
    -- end

    
    -- obj1.vel.x = v1.x - v1_in_dr*dr.x;
    -- obj1.vel.y = v1.y - v1_in_dr*dr.y;
    -- obj2.vel.x = v2.x - v2_in_dr*dr.x;
    -- obj2.vel.y = v2.y - v2_in_dr*dr.y;
end


function EnemyWallCollision(obj1, obj2)
    local dr = Vec(obj2.x - obj1.x, obj2.y - obj1.y);
    
end

function PlayerBulletCollision(obj1, obj2)
    if obj2.target.id == obj1.id then
        obj1.health = obj1.health - 1;
        obj2.kill(obj2);
    end
end

function EnemyBulletCollision(obj1, obj2)
    if obj2.target.id == obj1.id then
        obj1.health = obj1.health - 1;
        obj2.kill(obj2);
    end
end

function EnemyEnemyCollision(obj1, obj2)

    -- local dr = Vec(obj2.x - obj1.x, obj2.y - obj1.y);
    -- local dist = norm(dr);


    RepulsePairForce(obj1, obj2); 

    -- local v1 = obj1.vel;
    -- local v1_in_dr = dot(v1, dr);
    -- local v2 = obj2.vel;
    -- local v2_in_dr = -dot(v2, dr);

    -- if dist < 8. then
    --     v1_in_dr = 0.;
    --     v2_in_dr = 0.;
    -- end

    -- obj1.vel.x = v1.x - v1_in_dr*dr.x;
    -- obj1.vel.y = v1.y - v1_in_dr*dr.y;
    -- obj2.vel.x = v2.x - v2_in_dr*dr.x;
    -- obj2.vel.y = v2.y - v2_in_dr*dr.y;

end

function ResolveCollision(obj1, obj2)
    -- obj1.health = obj1.health - 1;
    -- if obj1.type then
    --     -- print(obj1.type);
    --     -- if obj2.owner == obj1.id then -- do not hit itself
    --         -- return;
    --     -- end
    --     -- obj1.health = 0;
    -- end
end

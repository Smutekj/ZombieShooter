ENEMY        = 0
BULLET       = 1
PLAYER       = 2
WALL         = 3
ORBITER      = 4
VISUALEFFECT = 5



function ResolveCollision(obj1, obj2)

    obj1.health = obj1.health - 1;
    -- if obj1.type then
    --     -- print(obj1.type);
    --     -- if obj2.owner == obj1.id then -- do not hit itself
    --         -- return;
    --     -- end
    --     -- obj1.health = 0;
    -- end

end

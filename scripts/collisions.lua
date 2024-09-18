ENEMY        = 0
BULLET       = 1
PLAYER       = 2
WALL         = 3
ORBITER      = 4
VISUALEFFECT = 5



function resolveCollision(obj1, obj2)

    if (obj1.type == ENEMY and obj2.type == BULLET) then
        if obj2.owner == obj1.id then -- do not hit itself
            return;
        end
        obj1.health = obj1.health - 1;
    end

end

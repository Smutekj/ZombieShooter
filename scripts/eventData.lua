FireBlastEvent =
{
    playerCol = function(obj1, obj2)
        obj2.health = obj2.health - 1;
    end,
    
    wallCol = function(obj1, obj2) end,

    enemyCol = function(obj1, obj2)
        obj2.health = obj2.health - 1;
    end,

    update = function(event)
        if timer > 300 then
            event.kill(event);
        end
    end,

    timer = 0.,

}

Events = {}
Events["fireblast"] = FireBlastEvent;

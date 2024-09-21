

function EventPlayerCollision(obj1, obj2)

end

function EventWallCollision(obj1, obj2)

end

function EventEnemyCollision(obj1, obj2)
    obj2.health  = obj2.health - 1;
end

Timer = 0.;

function EventUpdate(event)
    Timer = Timer + 1.;
    
end
ENEMY        = 0
BULLET       = 1
PLAYER       = 2
WALL         = 3
ORBITER      = 4
VISUALEFFECT = 5


Quest =
{
    id = 0,
    entity_types = {ENEMY},
    name = "Default Quest",
    description = "Kill enemies!",
    logic = function(e) return false; end,
    onFinish = function() end,
    counter = 1
};


local function setContains(set, key)
    return set[key] ~= nil
end


function updateCounters(entity)
    if setContains(Quest.entity_types, 1) then
        print("GOT HERE!!!");
        -- Quest.counter = Quest.counter - 1;
        -- if Quest.counter <= 0 then
        --     print("QUEST FINISHED!");
        --     Quest.onFinish();
        --     return true;
        -- end
    end
    return false;
end

function onFinish()
    local enemy = createEnemy("Enemy" + tostring(1));
    local player = getObject("Player");
    enemy.target = player;
    enemy.pos = player.pos;
end

Quests =
{

}


Quest.logic = updateCounters;
Quest.onFinish = onFinish;
Quests[Quest.id] = Quest;

return 0;
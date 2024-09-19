


function OnEntityDead(entity)
    print(type(Quests))
    for id, quest in pairs(Quests) do
        print(type(quest.logic));
        local x = quest.logic(entity);
        if x == true then -- if quest is finished we remove him
    --         Quests[id] = nil;
        end
    end
end


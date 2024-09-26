
function UpdateTimers(dt)

    for id, cooldown in pairs(GlobalCooldowns) do -- iterate over all entities global cooldowns and update them
        if cooldown > 0 then
            GlobalCooldowns[id] = cooldown - dt;
        end
        
        -- update cooldowns of all abilities of the entity with current id
        for spell_id, ability in pairs(Entity2Abilities[id]) do
            if ability.timer > 0 then
                ability.timer = ability.timer - dt;
            end
        end
    end
end

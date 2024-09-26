


BasicAttackAbility = { spell_id = 0, cooldown = 1, timer = 0, name = "basicattack"}

BasicAttackAbility.onUse = function (performer, ability_id, target)
    
    local cooldowns = GlobalCooldowns[performer.id];

    if cooldowns.global_timer > 0 then
        return;
    end

    if cooldowns.ability_timers[ability_id] > 0 then
        return;
    end

    if dist(performer.pos, target.pos) < 50 then
        cooldowns.timer = 2;
        target.health = target.health - 1;
    end
end
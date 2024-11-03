MeleeAttack = { spell_id = 3, range = 50, cooldown = 1, cast_time = 0., timer = 0, name = "melee" }

MeleeAttack.onUse = function(performer, target)
    -- check Cooldowns
    if GlobalCooldowns[performer.id] > 0.001 then
        return;
    end
    if Entity2Abilities[performer.id][MeleeAttack.spell_id].timer > 0.001 then
        return;
    end
    
    target.health = target.health - 2;
    performer.motion_state = MotionState.ATTACKING; -- attacking

    GlobalCooldowns[performer.id] = 1.; -- reset cooldown
    Entity2Abilities[performer.id][MeleeAttack.spell_id].timer = MeleeAttack.cooldown;
end



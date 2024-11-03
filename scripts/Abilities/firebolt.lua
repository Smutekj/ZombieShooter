

FireBoltAbility = { spell_id = 1, cooldown = 5, cast_time = 2.,timer = 0, name = "firebolt"}

FireBoltAbility.onUse = function (performer, target)

    -- check Cooldowns
    if GlobalCooldowns[performer.id] > 0.001 then
        return;
    end
    if Entity2Abilities[performer.id][FireBoltAbility.spell_id].timer > 0.001 then
        return;
    end

    performer.motion_state = MotionState.CASTING;  

    local p = createProjectile("Bullet", "FireBolt" .. tostring(performer.id));
    p.x = performer.x;
    p.y = performer.y;
    p.owner = performer.id;
    p.vel.x = target.x - performer.x;
    p.vel.y = target.y - performer.y;
    p.scale = Vec(10, 10);
    -- setScale("Firebolt" .. tostring(performer.id), Vec(20., 20.));
    -- setScale("Bullet", 100., 100.);
    p.target = target;
    -- p.target_pos.x= target.x;
    -- p.target_pos.y= target.y;
    performer.vel.x = 0;
    performer.vel.y = 0;
    p.setScript(p, "firebolt.lua");

    GlobalCooldowns[performer.id] = 2.; -- reset cooldown
    Entity2Abilities[performer.id][FireBoltAbility.spell_id].timer = FireBoltAbility.cooldown;

end

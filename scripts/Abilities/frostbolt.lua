


FrostBoltAbility = { spell_id = 2, cooldown = 3, timer = 0, name = "frostbolt"}

FrostBoltAbility.onUse = function (performer, target) 
    local p = createProjectile("Bullet", "P");
    p.x = performer.x;
    p.y = performer.y;
    p.owner = performer.id;
    p.vel.x = target.x - performer.x;
    p.vel.y = target.y - performer.y;
    p.target = target
    performer.vel.x = 0;
    performer.vel.y = 0;
    p.setScript(p, "frostbolt.lua");

    local cooldowns = GlobalCooldowns[performer.id];
    cooldowns.timer = 2.;
    
    Entity2Abilities[performer.id][Ability.spell_id].cooldown = FrostBoltAbility.cooldown;
end

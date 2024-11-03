ConsecrationAbility = { spell_id = 5, cooldown = 25, cast_time = 1.,timer = 0, name = "consecration"}

ConsecrationAbility.onUse = function (performer, target)

    -- check Cooldowns
    if GlobalCooldowns[performer.id] > 0.001 then
        return;
    end
    if Entity2Abilities[performer.id][ConsecrationAbility.spell_id].timer > 0.001 then
        return;
    end

    print(Events["consecration"].timer)

    local ev = createEvent("consecration");
    local eff = createEffect("consecration");

    ev.x = performer.x;
    ev.y = performer.y;
    ev.owner = performer.id;
    ev.scale = Vec(25, 25);
    eff.x = performer.x;
    eff.y = performer.y;
    eff.owner = performer.id;
    eff.scale = Vec(25, 25);
    setPosition("consecration", mouse_pos.x, mouse_pos.y);



    GlobalCooldowns[performer.id] = 2.; -- reset cooldown
    Entity2Abilities[performer.id][ConsecrationAbility.spell_id].timer = ConsecrationAbility.cooldown;

end
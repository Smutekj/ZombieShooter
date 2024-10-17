


SelectedSpellId = 0;


function DoAbility(mouse_pos, target_pos)

    local player = getPlayer("Player#0");
    print(ConsecrationAbility.spell_id)
    print(player.id)
    Entity2Abilities[player.id][ConsecrationAbility.spell_id].onUse(player, player.target);
end
    


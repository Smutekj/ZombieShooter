


-- initialize abilities
-- Entity2Abilities[player_id][FireBoltAbility.spell_id] = FireBoltAbility;
-- Entity2Abilities[player_id][BasicAttackAbility.spell_id] = BasicAttackAbility;
-- Entity2Abilities[player_id][FrostBoltAbility.spell_id] = FrostBoltAbility;


PlayerData = {

}


PlayerData.initialize = function (player)
    -- initialize abilities
    Entity2Abilities[player.id][FireBoltAbility.spell_id] = FireBoltAbility;
    Entity2Abilities[player.id][BasicAttackAbility.spell_id] = BasicAttackAbility;
    Entity2Abilities[player.id][FrostBoltAbility.spell_id] = FrostBoltAbility;
end



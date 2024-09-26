



EnemyData.enemy1 = {};

EnemyData.enemy1.initialize = function (enemy)
    -- initialize abilities
    print(Entity2Abilities[enemy.id]);
    
    Entity2Abilities[enemy.id][FireBoltAbility.spell_id] = FireBoltAbility;
    print("INITIALIZING ENEMY!!!");
end




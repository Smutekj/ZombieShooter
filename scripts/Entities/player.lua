


-- initialize abilities
-- Entity2Abilities[player_id][FireBoltAbility.spell_id] = FireBoltAbility;
-- Entity2Abilities[player_id][BasicAttackAbility.spell_id] = BasicAttackAbility;
-- Entity2Abilities[player_id][FrostBoltAbility.spell_id] = FrostBoltAbility;


PlayerData = {}


PlayerData.initialize = function (player)
    -- initialize abilities
    Entity2Abilities[player.id][FireBoltAbility.spell_id] = FireBoltAbility;
    Entity2Abilities[player.id][BasicAttackAbility.spell_id] = BasicAttackAbility;
    Entity2Abilities[player.id][FrostBoltAbility.spell_id] = FrostBoltAbility;
end



local function drawAgent(enemy, layers)
    local texture = getTexture("coin");
    local sprite = Sprite();
    sprite.setTexture(sprite, 0, texture);
    sprite.pos = Vec(enemy.x, enemy.y);
    sprite.scale = Vec(20., 20.);

    sprite.angle = math.rad(-enemy.angle - 270);
    layers.drawSprite(layers, "Wall", sprite, "basicagent");

    local angle_rad = math.rad(enemy.angle);
    local dr = Vec(5*math.cos(angle_rad), 5*math.sin(angle_rad));
    layers.drawLine(layers, "Unit", enemy.pos, enemy.pos + dr, 1., Color(1,0,0,1));

end

function DrawPlayer(player, layers) --object is the c++ passed function
    
    if not layers.isActive(layers, "Light") then
        layers.toggleActivate(layers, "Light");
    end

    drawAgent(player, layers);
end



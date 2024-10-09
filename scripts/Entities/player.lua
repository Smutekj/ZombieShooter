-- initialize abilities
-- Entity2Abilities[player_id][FireBoltAbility.spell_id] = FireBoltAbility;
-- Entity2Abilities[player_id][BasicAttackAbility.spell_id] = BasicAttackAbility;
-- Entity2Abilities[player_id][FrostBoltAbility.spell_id] = FrostBoltAbility;


PlayerData = {}


PlayerData.initialize = function(player)
    -- initialize abilities
    Entity2Abilities[player.id][FireBoltAbility.spell_id] = FireBoltAbility;
    Entity2Abilities[player.id][BasicAttackAbility.spell_id] = BasicAttackAbility;
    Entity2Abilities[player.id][FrostBoltAbility.spell_id] = FrostBoltAbility;
end

local function rotate(vec, angle)
    local result = Vec(vec.x, vec.y);
    local phi = math.rad(angle);
    result.x = vec.x * math.cos(phi) + vec.y * math.sin(phi); 
    result.y = vec.x * math.sin(phi) - vec.y * math.cos(phi); 
    return result;
end


local function drawAgent(enemy, layers)
    local texture = getTexture("coin");
    local sprite = Sprite();
    sprite.setTexture(sprite, 0, texture);
    sprite.pos = Vec(enemy.x, enemy.y);
    sprite.scale = Vec(20., 20.);

    sprite.angle = math.rad(-enemy.angle - 270);
    layers.drawSprite(layers, "Wall", sprite, "basicagent");

    -- draw eyes
    -- local left_eye_pos = Vec(enemy.x - 6., enemy.y + 8.); -- left eye
    -- local right_eye_pos= Vec(enemy.x + 6., enemy.y + 8.);  -- right eye
    local left_eye_pos  = rotate(Vec(8., -6.), enemy.angle) + enemy.pos;
    local right_eye_pos = rotate(Vec( 8., 6.), enemy.angle) + enemy.pos;
    -- sprite.angle = 0.;
    sprite.scale = Vec(2.69, 2.69);
    sprite.pos = left_eye_pos;
    layers.drawSprite(layers, "Wall", sprite, "basicshield");
    sprite.pos = right_eye_pos
    layers.drawSprite(layers, "Wall", sprite, "basicshield");
    

    local angle_rad = math.rad(enemy.angle);
    local dr = Vec(5 * math.cos(angle_rad), 5 * math.sin(angle_rad));
    layers.drawLine(layers, "Wall", enemy.pos, enemy.pos + dr, 1., Color(3, 0, 0, 1)); -- draw nose
end

function DrawPlayer(player, layers) --object is the c++ passed function
    if not layers.isActive(layers, "Light") then
        layers.toggleActivate(layers, "Light");
    end

    drawAgent(player, layers);
end

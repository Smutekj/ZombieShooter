
local function randVec()
    local rand_angle = math.random() * 2. * math.pi;
    return Vec(math.cos(rand_angle), math.sin(rand_angle));
end

local function randVec(size)
    local rand_angle = math.random() * 2. * math.pi;
    local result = Vec(0, 0);
    result.x = size * math.cos(rand_angle);
    result.y = size * math.sin(rand_angle);
    return result;
end

Effect = {}
-- Effect["Particles1"] = Particles1;
-- Effect["Particles2"] = Particles2;

local function drawTheThing(position, layers, radius)
    local thickness = radius / 10.;
    local prev_pos = Vec(position.x + radius, position.y);

    local texture = getTexture("coin");
    local sprite = Sprite();
    sprite.setTexture(sprite, 0, texture);
    sprite.pos = position;
    sprite.scale = Vec(200., 200.);
    layers.drawSprite(layers, "Wall", sprite, "consecration");

end

Effect.Drawer = function(effect, layers)
    local c = Color(1, 0, 0, 1);
    local eye_color = Color(10, 0, 0, 1); 
    drawTheThing(effect.pos, layers, 10.);
end

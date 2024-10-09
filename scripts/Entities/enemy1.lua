EnemyData.enemy1 = { vision_range = 300., attack_range = 20. };

EnemyData.enemy1.initialize = function(enemy)
    -- initialize abilities
    print(Entity2Abilities[enemy.id]);

    Entity2Abilities[enemy.id][FireBoltAbility.spell_id] = FireBoltAbility;
    -- Entity2Draw[enemy.id] = ;
    print("INITIALIZING ENEMY!!!");

    
end


local function drawAgent(enemy, layers, radius, color, eye_color)
    local texture = getTexture("coin");
    local sprite = Sprite();
    sprite.setTexture(sprite, 0, texture);
    sprite.pos = Vec(enemy.x, enemy.y);
    sprite.scale = Vec(15., 15.);
    layers.drawSprite(layers, "Wall", sprite, "basicagent");

    local vision_circle = Sprite();
    vision_circle.setTexture(vision_circle, 0, texture);
    vision_circle.pos = enemy.pos;
    vision_circle.scale = Vec(EnemyData.enemy1.vision_range, EnemyData.enemy1.vision_range);
    vision_circle.angle = enemy.angle;


    -- layers.drawSprite(layers, "Wall", vision_circle, "circle");
    -- vision_circle.scale = Vec(EnemyData.enemy1.attack_range, EnemyData.enemy1.attack_range);
    -- layers.drawSprite(layers, "Wall", vision_circle, "circle");
end


local function drawHealthBar(enemy, canvas)
    local size = canvas.getSize(canvas);
    local texture = getTexture("coin");
    local health_bar = Sprite();
    health_bar.setTexture(health_bar, 0, texture);
    health_bar.pos = enemy.pos + Vec(0., 50.);
    health_bar.scale = Vec(50., 8.);
    canvas.drawSprite(canvas, health_bar, "healthBar"); -- DO NOT CHANGE ORDER OF THESE!

    local x_scale = enemy.health / enemy.max_health;
    local resources_info = Sprite();
    resources_info.setTexture(resources_info, 0, texture);
    resources_info.pos = health_bar.pos + Vec(-50.*(1. - x_scale),  0.);
    resources_info.scale = Vec(100. * x_scale, 15.5);
    canvas.drawRectangle(canvas, resources_info, "default", Color(1. - x_scale, x_scale, 0., 1.));
end
    -- local health_text = Text(tostring(player.health) .. " / " .. tostring(player.max_health));
    -- local font = getFont();
    -- health_text.setFont(health_text, font);
    -- health_text.pos = enemy.pos + Vec(health_bar.pos.x - 50 - 30, health_bar.pos.y - 12.5);
    -- health_text.scale = Vec(1, 1);
    -- canvas.render(canvas);
    -- canvas.drawText(canvas, health_text, "Text")

function DrawEnemy(enemy, layers) --object is the c++ passed function
    if not layers.isActive(layers, "Light") then
        layers.toggleActivate(layers, "Light");
    end
    drawHealthBar(enemy, layers.getCanvas(layers, "UI"));

    local player = getObject("Player");
    local c = Color(1, 0, 0, 1);
    local eye_color = Color(0, 10, 0, 1);
    if (enemy.state == ATTACKING) then
        eye_color = Color(90, 0, 0, 1);
    end
    drawAgent(enemy, layers, 10, c, eye_color);
    -- for i = 0,n_enemies-1 do



    --     local id = enemies:at(i).id;
    --     -- drawAgent(enemies:at(i), layers, 10., c, eye_color);
    --     -- Entity2Draw[id].draw(enemies:at(i), layers);
    -- end
end

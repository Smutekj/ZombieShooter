

local function drawHealthBar(player, canvas)
    local size = canvas.getSize(canvas);
    local texture = getTexture("coin");
    local health_bar = Sprite();
    health_bar.setTexture(health_bar, 0, texture);
    health_bar.pos = Vec(0.1 * size.x, 0.9 * size.y);
    health_bar.scale = Vec(100., 25.);
    canvas.drawSprite(canvas, health_bar, "healthBar"); -- DO NOT CHANGE ORDER OF THESE!
    
    local x_scale = player.health / player.max_health;
    local resources_info = Sprite();
    resources_info.setTexture(resources_info, 0, texture);
    resources_info.pos = Vec(0.1 * size.x - 195.*(1. - x_scale)/2. , 0.9 * size.y );
    resources_info.scale = Vec(195.*x_scale, 45.);
    canvas.drawRectangle(canvas, resources_info, "default", Color(0.,1.,1.,1.));

    local health_text = Text(tostring(player.health) .. " / " .. tostring(player.max_health));
    local font = getFont();
    health_text.setFont(health_text, font);
    health_text.pos = Vec(health_bar.pos.x - 50 - 30, health_bar.pos.y - 12.5);
    health_text.scale = Vec(1, 1);
    canvas.render(canvas);
    canvas.drawText(canvas, health_text, "Text")
end

local function drawCastBar(player, canvas)
    local size = canvas.getSize(canvas);
    local texture = getTexture("coin");
    local health_bar = Sprite();
    health_bar.setTexture(health_bar, 0, texture);
    health_bar.pos = Vec(0.1 * size.x, 0.9 * size.y);
    health_bar.scale = Vec(100., 25.);
    canvas.drawSprite(canvas, health_bar, "healthBar"); -- DO NOT CHANGE ORDER OF THESE!
    
    local x_scale = player.health / player.max_health;
    local resources_info = Sprite();
    resources_info.setTexture(resources_info, 0, texture);
    resources_info.pos = Vec(0.1 * size.x - 195.*(1. - x_scale)/2. , 0.9 * size.y );
    resources_info.scale = Vec(195.*x_scale, 45.);
    canvas.drawRectangle(canvas, resources_info, "default", Color(0.,1.,0.,1.));

    local health_text = Text(tostring(player.health) .. " / " .. tostring(player.max_health));
    local font = getFont();
    health_text.setFont(health_text, font);
    health_text.pos = Vec(health_bar.pos.x - 50 - 30, health_bar.pos.y - 12.5);
    health_text.scale = Vec(1, 1);
    canvas.render(canvas);
    canvas.drawText(canvas, health_text, "Text")
end

local function drawTarget(player, target, canvas)
    local size = canvas.getSize(canvas);
    local texture = getTexture("coin");
    local health_bar = Sprite();
    health_bar.setTexture(health_bar, 0, texture);
    health_bar.pos = Vec(0.4 * size.x, 0.9 * size.y);
    health_bar.scale = Vec(100., 25.);
    canvas.drawSprite(canvas, health_bar, "healthBar"); -- DO NOT CHANGE ORDER OF THESE!
    canvas.render(canvas);

    local x_scale = target.health / target.max_health;
    local resources_info = Sprite();
    resources_info.setTexture(resources_info, 0, texture);
    resources_info.pos = Vec(0.4 * size.x - 195.*(1. - x_scale)/2. , 0.9 * size.y );
    resources_info.scale = Vec(195.*x_scale, 45.);
    canvas.drawRectangle(canvas, resources_info, "default", Color(0.,1.,0.,1.));
    
    local health_text = Text(tostring(target.health) .. " / " .. tostring(target.max_health));
    local font = getFont();
    health_text.setFont(health_text, font);
    health_text.pos = Vec(health_bar.pos.x - 50 - 30, health_bar.pos.y - 12.5);
    health_text.scale = Vec(1, 1);
    canvas.render(canvas);
    canvas.drawText(canvas, health_text, "Text")
end

local function drawEnemyBar(enemy, canvas, layers)

end

TargetID = -1;

function DrawUI(player, canvas, layers)
    drawHealthBar(player, canvas);
    drawCastBar(player, canvas);
    
    local target = player.target_enemy;
    if not (target == nil) and not (target == 0) then        
        TargetID = target.id;
        drawTarget(player, player.target_enemy, canvas);
    end 
    -- print(TargetID, "HI")

    -- if (not target == 0) then
    -- end
    player.vision_radius = 900.;

    -- local enemies = findEnemies(player.pos, 500.);
    -- local n_enemies = enemies.size(enemies);
    -- for i = 0,n_enemies-1 do
        -- print(enemies:at(i));
    -- end
end

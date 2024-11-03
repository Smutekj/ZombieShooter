
for i = 1, 20 do -- creates enemies at random position around the map
    local name = "E" .. tostring(i);
    local enemy = createEnemy(name);
    local map_size = getMapSize();
    local new_pos = Vec(map_size.x/2, map_size.y/2) + randVec(math.random() * map_size.x/2*0.95);
    local new_target = Vec(map_size.x/2, map_size.y/2) + randVec(math.random() * map_size.x/2*0.95);
    print(new_pos.x, new_pos.y)
    enemy.pos = new_pos;
    enemy.target_pos = new_target;
    print("wtf");
    print("Created: " .. name);
end

player_p = getPlayer("Player#0")
print(player_p.max_vel)
player_p.max_vel = 300.;
player_p.vision_radius = 600.;

return 0;

for i = 1, 5 do
    local name = "E" .. tostring(i);
    local enemy = createEnemy(name);
    local new_pos = Vec(2500, 2500) + randVec(math.random() * 2500.);
    local new_target = Vec(2500, 2500) + randVec(math.random() * 2500.);
    print(new_pos.x, new_pos.y)
    enemy.pos = new_pos;
    enemy.target_pos = new_target;
    print("wtf");
    print("Created: " .. name);
end

player_p = getPlayer("Player")
print(player_p.max_vel)
player_p.max_vel = 300.;
player_p.vision_radius = 600.;

return 0;



for i =1,1 do
    local name = "E" .. tostring(i);
    local enemy = createEnemy(name);
    setPosition(name, 500,500);
end


return 0;
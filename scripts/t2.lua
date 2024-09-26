

for i =10,1000 do
    local name = "PenisDenis" .. tostring(i);
    local effect = createEffect(name, "testeffect.lua");
    local status = setPosition(name, 400, 500);
    changeParentOf(name, "Player")
    -- print(status);
end


return 0;
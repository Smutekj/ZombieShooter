Events = {}

local f, error = loadfile( "../scripts/Events/consecration.lua")
if f then f() else print(error) end

print("INIT EVENTS")

return 0;
Effects = {}

local f, error = loadfile( "../scripts/Effects/consecration.lua")
if f then f() else print(error) end


return 0;
GlobalCooldowns = {}  -- we store GlobalCooldowns of all entities here


AbilityCooldowns = {} -- we store cooldowns of all abilities here
Entity2Abilities = {} -- we store here ids ofof abilities each entity has available (like a spellbook)
Entity2Draw = {}
Entity2SelectedAbility = {}

Ability2Data = {};

local Ability = { id = 0, cooldown = 1, cast_time = 0., timer = 0, name = "DefaultAbility" }

EnemyData = {

}


-- initialize spell books
local f2, error2 = loadfile( "../scripts/Entities/enemy1.lua")
if f2 then f2()  else print(error2) end
local f, error = loadfile( "../scripts/Entities/player.lua")
if f then f() else print(error) end

local f, error = loadfile( "../scripts/Abilities/basicattack.lua")
if f then f() else print(error) end
local f3, error3 = loadfile( "../scripts/Abilities/firebolt.lua")
if f3 then f3() else print(error3) end
local f, error = loadfile( "../scripts/Abilities/frostbolt.lua")
if f then f() else print(error) end
local f, error = loadfile( "../scripts/Abilities/consecration.lua")
if f then f() else print(error) end

return 0;
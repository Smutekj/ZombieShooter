function dump(o)
   if type(o) == 'table' then
      local s = '{ ';
      for k, v in pairs(o) do
         if type(k) ~= 'number' then k = '"' .. k .. '"' end
         s = s .. '[' .. k .. '] = ' .. dump(v) .. ',';
      end
      return s .. '} '
   else
      return tostring(o);
   end
end

function OnEntityDead(entity)

   local player = getPlayer("Player#0");
   
   -- local target = player.target_enemy;
   -- if target.id == entity.id then
   --    player.target_enemy = nil;
   -- end
   -- for id, quest in pairs(Quests) do
      
   --    local x = quest.logic(entity);
   --    if x == true then -- if quest is finished we remove him
   -- --         Quests[id] = nil;
   --          print("quest complete!");
   --    end
   -- end
end

function OnEnemyCreation(enemy, enemy_type)

   Entity2Abilities[enemy.id] = {};
   GlobalCooldowns[enemy.id] = 1.;
   ChaseTimers[enemy.id] = 0.;

   Entity2SelectedAbility[enemy.id] = 3;
   EnemyData[enemy_type].initialize(enemy);
end

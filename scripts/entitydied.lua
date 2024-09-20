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
   -- for id, quest in pairs(Quests) do
      
   --    local x = quest.logic(entity);
   --    if x == true then -- if quest is finished we remove him
   -- --         Quests[id] = nil;
   --          print("quest complete!");
   --    end
   -- end
end

function copy(obj, seen)
    if type(obj) ~= 'table' then return obj end
    if seen and seen[obj] then return seen[obj] end
    local s = seen or {}
    local res = setmetatable({}, getmetatable(obj))
    s[obj] = res
    for k, v in pairs(obj) do res[copy(k, s)] = copy(v, s) end
    return res
  end
  


FireStatus =
{
    update = function(affectee)
        local timer = FireStatus.timer;
        if timer > FireStatus.duration then
            timer = 0.;
        end
        timer = timer + 1.;
        FireStatus.timer = 0.;
    end,

    timer = 0.,
    duration = 300.,
}



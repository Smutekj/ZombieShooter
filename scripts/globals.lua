
Timers = {}
ChaseTimers = {}



function dot(v1, v2)
    return v1.x * v2.x + v1.y * v2.y;
end

function norm2(v1)
    return dot(v1, v1);
end

function norm(v1)
    return math.sqrt(norm2(v1));
end

function dist(v1, v2)
    local dr = Vec(v1.x - v2.x, v1.y - v2.y);
    return norm(dr);
end

function normalize(v1)
    local n = norm(v1);
    v1.x = v1.x / n;
    v1.y = v1.y / n;
end

function randVec()
    rand_angle = 2.*math.random()*math.pi;
    return Vec(math.cos(rand_angle), math.sin(rand_angle));
end

function randVec(size)
    rand_angle = 2.*math.random()*math.pi;
    return Vec(size*math.cos(rand_angle), size*math.sin(rand_angle));
end

return 0;



#pragma once

#include <random>

#include "../Vertex.h"

inline float randf(const float min = 0, const float max = 1)
{
    return (rand() / static_cast<float>(RAND_MAX)) * (max - min) + min;
}

inline Vec2 randomPosInBox(Vec2 ul_corner,
                            Vec2 box_size)
{
    return {ul_corner.x + rand() / static_cast<float>(RAND_MAX) * box_size.x,
            ul_corner.y + rand() / static_cast<float>(RAND_MAX) * box_size.y};
}

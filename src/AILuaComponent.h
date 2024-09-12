#pragma once

#include <Vector2.h>

    enum class AIState
    {
        Patroling,
        Chasing,
        Attacking,
    };

struct AgentLuaComponent
{
    AIState state = AIState::Patroling;
    utils::Vector2f pos = {0,0};
    utils::Vector2f target_pos;
    utils::Vector2f path_pos;
    utils::Vector2f v;
};

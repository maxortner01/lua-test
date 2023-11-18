#pragma once

#include "../Lua.hpp"

namespace S2D::Engine
{
    struct Input : Lua::Lib::Base
    {
        enum class KeyState
        {
            Press   = 0b001,
            Down    = 0b010,
            Release = 0b100
        };

        inline static std::unordered_map<char, KeyState> global_state;
        static int getPressed(Lua::State L);
        static int getDown(Lua::State L);
        static int getReleased(Lua::State L);
        Input();
    };
}
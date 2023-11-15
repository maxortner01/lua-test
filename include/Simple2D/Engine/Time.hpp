#pragma once

#include "../Lua.hpp"

namespace S2D::Engine
{
    struct Time : Lua::Lib::Base
    {
        inline static double dt;
        static int deltaTime(Lua::State L);
        Time();
    };
}
#pragma once

#include "../Lua.hpp"

namespace S2D::Engine
{
    struct Time : Lua::Lib::Base
    {
        static int deltaTime(Lua::State L);
        Time();
    };
}
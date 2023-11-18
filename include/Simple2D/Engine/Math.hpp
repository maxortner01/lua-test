#pragma once

#include "../Lua.hpp"

namespace S2D::Engine
{
    struct Math : Lua::Lib::Base
    {
        static int normalize(Lua::State L);
        Math();
    };
}
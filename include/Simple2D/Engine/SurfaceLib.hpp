#pragma once

#include "../Lua.hpp"

namespace S2D::Engine
{
    struct SurfaceLib : Lua::Lib::Base
    {
        static int drawText(Lua::State L);
        SurfaceLib();
    };
}

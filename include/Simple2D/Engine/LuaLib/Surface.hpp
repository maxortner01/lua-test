#pragma once

#include "../../Lua.hpp"

namespace S2D::Engine
{
    struct Surface : Lua::Lib::Base
    {
        static int drawText(Lua::State L);
        Surface();
    };
}

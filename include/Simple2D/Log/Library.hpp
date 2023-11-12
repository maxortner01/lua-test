#pragma once

#include "../Lua/Lib.hpp"

namespace S2D::Log
{
    struct Library : Lua::Lib::Base
    {
        Library();
        ~Library() = default;

        static int log(Lua::State L);
        static int _assert(Lua::State L);
        static int error(Lua::State L);
    };
}
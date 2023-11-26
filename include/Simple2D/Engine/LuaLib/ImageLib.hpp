#pragma once 

#include "../../Lua.hpp"

namespace S2D::Engine
{
    struct ImageLib : Lua::Lib::Base
    {
        static int getSize(Lua::State L);
        static int getPixel(Lua::State L);
        ImageLib();
    };
}
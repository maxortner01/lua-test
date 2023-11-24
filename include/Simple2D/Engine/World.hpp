#pragma once

#include "../Lua.hpp"

namespace S2D::Engine
{
    struct World : Lua::Lib::Base
    {
        static int createEntity(Lua::State L);
        static int getEntity(Lua::State L);
        
        World();
    };
}
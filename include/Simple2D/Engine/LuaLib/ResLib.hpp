#pragma once

#include "../../Lua.hpp"

namespace S2D::Engine
{
    enum class ResourceType
    {
        Texture, Font, Image, Surface, Count
    };

    const char* operator*(ResourceType r);

    struct ResLib : Lua::Lib::Base
    {
        static int getResource(Lua::State L);

        ResLib();
    };
}
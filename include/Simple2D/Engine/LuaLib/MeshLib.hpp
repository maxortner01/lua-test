#pragma once

#include "../../Lua.hpp"

namespace S2D::Engine
{

    struct MeshLib : Lua::Lib::Base
    {
        static int setPrimitiveType(Lua::State L);
        static int getVertexCount(Lua::State L);
        static int pushVertex(Lua::State L);
        static int getVertex(Lua::State L);

        MeshLib();
    };

}
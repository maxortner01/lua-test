#pragma once

#include "Shader.hpp"
#include "Texture.hpp"

namespace S2D::Graphics
{
    struct Context
    {
        Program* program;
        std::vector<const Texture*> textures;
    };
}
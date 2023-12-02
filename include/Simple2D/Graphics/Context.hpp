#pragma once

#include "Shader.hpp"
#include "Texture.hpp"

namespace S2D::Graphics
{
    struct Context
    {
        Context() : program(nullptr)
        {   }

        ~Context() = default;

        Program* program;
        std::vector<const Texture*> textures;
    };
}
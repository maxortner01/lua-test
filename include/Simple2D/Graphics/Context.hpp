#pragma once

#include "Shader.hpp"
#include "Texture.hpp"

namespace S2D::Graphics
{
    struct Context
    {
        Context() : program(nullptr), depth_test(true)
        {   }

        ~Context() = default;

        Program* program;
        std::vector<const Texture*> textures;
        bool depth_test;
    };
}
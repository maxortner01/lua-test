#pragma once

#include "../Def.hpp"

#include "Vector.hpp"

namespace S2D::Graphics
{
    struct Color
    {
        float r, g, b, a;

        Color(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) :
            r(_r / 255.f), g(_g / 255.f), b(_b / 255.f), a(_a / 255.f)
        {   }

        Math::Vec4f normalized() const
        {
            return Math::Vec4f {
                .x = (float)r / 255.f,
                .y = (float)g / 255.f,
                .z = (float)b / 255.f,
                .w = (float)a / 255.f
            };
        }
    };
}
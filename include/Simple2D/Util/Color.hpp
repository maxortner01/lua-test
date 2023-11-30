#pragma once

#include "../Def.hpp"

#include "Vector.hpp"

namespace S2D::Graphics
{
    struct Color
    {
        uint8_t r, g, b, a;

        Color(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) :
            r(_r), g(_g), b(_b), a(_a)
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
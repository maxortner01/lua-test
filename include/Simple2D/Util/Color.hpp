#pragma once

#include "../Def.hpp"

#include "Vector.hpp"

namespace S2D::Graphics
{
    struct Color
    {
        float r, g, b, a;

        Color(uint8_t _r = 255, uint8_t _g = 255, uint8_t _b = 255, uint8_t _a = 255) :
            r(_r / 255.f), g(_g / 255.f), b(_b / 255.f), a(_a / 255.f)
        {   }
    };
}
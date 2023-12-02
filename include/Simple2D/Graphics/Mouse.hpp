#pragma once

#include "../Util/Vector.hpp"

namespace S2D::Graphics
{
    struct Mouse
    {
        enum class Button
        {
            None,
            Left, Right, Middle
        };

        static Math::Vec2f getPosition();
    };

    const char* operator*(Mouse::Button button);
}
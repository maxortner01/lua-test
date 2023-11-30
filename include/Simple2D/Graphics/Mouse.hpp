#pragma once

namespace S2D::Graphics
{
    struct Mouse
    {
        enum class Button
        {
            None,
            Left, Right, Middle
        };
    };

    const char* operator*(Mouse::Button button);
}
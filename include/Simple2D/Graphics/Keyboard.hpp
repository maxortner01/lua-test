#pragma once

namespace S2D::Graphics
{
    struct Keyboard
    {
        enum class Key
        {
            None,
            W, A, S, D, Escape
        };
    };

    const char* operator*(Keyboard::Key key);
}
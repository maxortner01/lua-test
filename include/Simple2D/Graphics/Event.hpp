#pragma once

#include "Keyboard.hpp"
#include "Mouse.hpp"
#include "../Util/Vector.hpp"

namespace S2D::Graphics
{
    struct Event
    {
        Event() : mousePress{}
        {   }

        enum class Type
        {
            Unsupported,
            KeyPress,
            KeyRelease,
            MousePress,
            MouseRelease,
            Close
        } type;

        struct KeyEvent
        {
            Keyboard::Key key;
        };

        struct MouseButtonEvent
        {
            MouseButtonEvent() = default;

            Mouse::Button button;
            Math::Vec2f position;
        };

        union
        {
            KeyEvent     keyPress;
            KeyEvent     keyRelease;
            MouseButtonEvent mousePress;
            MouseButtonEvent mouseRelease;
        };
    };
}
#include <Simple2D/Graphics/Mouse.hpp>

namespace S2D::Graphics
{
    const char* operator*(Mouse::Button button)
    {
        switch (button)
        {
        case Mouse::Button::Left:   return "MouseLeft";
        case Mouse::Button::Right:  return "MouseRight";
        case Mouse::Button::Middle: return "MouseMiddle";
        default: return "";
        }
    }
}
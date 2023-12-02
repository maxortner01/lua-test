#include <Simple2D/Graphics/Mouse.hpp>

#include <SDL3/SDL.h>

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

    Math::Vec2f Mouse::getPosition()
    {
        Math::Vec2f pos;
        SDL_GetMouseState(&pos.x, &pos.y);
        return pos;
    }
}
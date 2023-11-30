#include <Simple2D/Graphics/Keyboard.hpp>

namespace S2D::Graphics
{
    const char* operator*(Keyboard::Key key)
    {
        switch (key)
        {
        case Keyboard::Key::W: return "W";
        case Keyboard::Key::A: return "A";
        case Keyboard::Key::S: return "S";
        case Keyboard::Key::D: return "D";
        case Keyboard::Key::Escape: return "Escape";
        default: return "";
        }
    }
}
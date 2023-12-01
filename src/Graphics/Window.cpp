#include <Simple2D/Graphics/Window.hpp>

#include <SDL3/SDL.h>

namespace S2D::Graphics
{
    Window::Window(
        const Math::Vec2u& size,
        const std::string& title) :
            _open(false),
            _size(size)
    {
        S2D_ASSERT(size.x && size.y, "Size is zero");
        S2D_ASSERT(SDL_Init(SDL_INIT_VIDEO) >= 0, "SDL Failed to initialize");

        window = SDL_CreateWindow(title.c_str(), size.x, size.y, SDL_WINDOW_OPENGL);
        S2D_ASSERT(window, "Window failed to create");

        _open = true;
    }

    const Math::Vec2u& Window::getSize() const
    {
        return _size;
    }

    Window::~Window()
    {
        SDL_DestroyWindow(reinterpret_cast<SDL_Window*>(window));
        window = nullptr;
    }

    Keyboard::Key handle_key(const SDL_Keysym& sym)
    {
        switch (sym.sym)
        {
        case SDLK_ESCAPE: return Keyboard::Key::Escape;
        case SDLK_w:      return Keyboard::Key::W;
        case SDLK_s:      return Keyboard::Key::S;
        case SDLK_a:      return Keyboard::Key::A;
        case SDLK_d:      return Keyboard::Key::D;
        default: return Keyboard::Key::None;
        }
    }

    Mouse::Button handle_mouse_button(uint8_t button)
    {
        switch (button)
        {
        case SDL_BUTTON_LEFT:   return Mouse::Button::Left;
        case SDL_BUTTON_RIGHT:  return Mouse::Button::Right;
        case SDL_BUTTON_MIDDLE: return Mouse::Button::Middle;
        default: return Mouse::Button::None;
        }
    }
    
    void Window::close()
    {
        _open = false;
    }

    bool Window::isOpen() const
    {
        return _open;
    }

    bool Window::pollEvent(Event& event) const
    {
        SDL_Event e;
        return (SDL_PollEvent(&e) ? [&]() -> bool
        {
            switch (e.type)
            {
            case SDL_EVENT_KEY_DOWN:
            {
                event.type = Event::Type::KeyPress;
                event.keyPress.key = handle_key(e.key.keysym);
                break;
            }
            case SDL_EVENT_KEY_UP:
            {
                event.type = Event::Type::KeyRelease;
                event.keyPress.key = handle_key(e.key.keysym);
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                event.type = Event::Type::MousePress;
                event.mousePress.button = handle_mouse_button(e.button.button);
                event.mousePress.position = { e.button.x, e.button.y };
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                event.type = Event::Type::MouseRelease;
                event.mouseRelease.button = handle_mouse_button(e.button.button);
                event.mousePress.position = { e.button.x, e.button.y };
                break;
            }
            case SDL_EVENT_QUIT:
            {
                event.type = Event::Type::Close;
                break;
            }
            default:
            {
                event.type = Event::Type::Unsupported;
                break;
            }
            }

            return true;
        }() : false);
    }
}
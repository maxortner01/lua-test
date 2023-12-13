#pragma once

#include "../Util/Vector.hpp"

#include "Event.hpp"

namespace S2D::Graphics
{
    struct Window
    {
        using Handle = void*;

        Window(const Math::Vec2u& size, const std::string& title);
        Window(const Window&) = delete;
        Window(Window&&) = delete;
        virtual ~Window();

        void close();
        bool isOpen() const;
        virtual bool pollEvent(Event& event) const;
    
    protected:
        Handle window;

        void standardEvents(void* sdl_event, Event& event) const;

    private:
        bool _open;
    };
}
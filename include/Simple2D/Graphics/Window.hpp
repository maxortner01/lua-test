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
        bool pollEvent(Event& event) const;

        const Math::Vec2u& getSize() const;
    
    protected:
        Handle window;

    private:
        bool _open;
        Math::Vec2u _size;
    };
}
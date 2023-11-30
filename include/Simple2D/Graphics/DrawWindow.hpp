#pragma once

#include "Window.hpp"

#include "../Util/Color.hpp"

namespace S2D::Graphics
{
    struct DrawWindow : Window
    {
        DrawWindow(const Math::Vec2u& size, const std::string& title);
        ~DrawWindow();

        void clear(const Color& color = Color(0, 0, 0, 255)) const;
        void display() const;

    private:
        Window::Handle context;
    };
}
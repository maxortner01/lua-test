#pragma once

#include "Window.hpp"
#include "Context.hpp"
#include "Drawable.hpp"
#include "Surface.hpp"

#include "../Util/Color.hpp"

namespace S2D::Graphics
{
    struct DrawWindow : Window, public Surface
    {
        DrawWindow(const Math::Vec2u& size, const std::string& title);
        ~DrawWindow();

        void display() const;
        void bind() const override;

    private:
        Window::Handle context;
    };
}
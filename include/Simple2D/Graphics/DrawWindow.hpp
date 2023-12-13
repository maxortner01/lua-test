#pragma once

#include "Window.hpp"
#include "Context.hpp"
#include "Drawable.hpp"
#include "Surface.hpp"

#include "../Util/Color.hpp"

namespace S2D::Graphics
{
    struct DrawTexture;

    struct DrawWindow : Window, public Surface
    {
        DrawWindow(const Math::Vec2u& size, const std::string& title);
        ~DrawWindow();

        void display() const;
        void bind() const override;

        void handle(const DrawTexture& tex);

        const Math::Vec2u& getSize() const override;

        bool pollEvent(Event& event) const override;

    private:
        Window::Handle context;
        Math::Vec2u _size;
    };
}
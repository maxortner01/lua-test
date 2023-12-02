#pragma once

#include "../Util/Color.hpp"

namespace S2D::Graphics
{
    struct Drawable;
    struct Context;

    struct Surface
    {
        virtual void bind() const = 0;

        void unbind() const;
        void clear(const Color& color = Color(0, 0, 0, 255)) const;
        void draw(const Drawable& object, const Context& context);

        virtual const Math::Vec2u& getSize() const = 0;
    };
}
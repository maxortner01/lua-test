#pragma once

namespace S2D::Graphics
{
    struct Drawable;
    struct Context;

    struct Surface
    {
        virtual void bind() = 0;
        void draw(const Drawable& object, const Context& context);
    };
}
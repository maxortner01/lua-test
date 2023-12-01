#pragma once

namespace S2D::Graphics
{
    struct Surface;
    struct Context;

    struct Drawable
    {
        // The object specific draw calls
        virtual void draw(Surface* window) const = 0;
    };
}
#pragma once

#include "../Util/Color.hpp"

namespace S2D::Graphics
{
    struct Drawable;
    struct Context;

    enum class LayerType : uint8_t
    {
        None  = 0b00,
        Color = 0b01,
        Depth = 0b10,
        Both  = 0b11
    };

    inline static constexpr LayerType operator|(LayerType a, LayerType b)
    {
        return (LayerType)((int)a | (int)b);
    }

    inline static constexpr LayerType operator&(LayerType a, LayerType b)
    {
        return (LayerType)((int)a & (int)b);
    }

    struct Surface
    {
        virtual void bind() const = 0;

        void unbind() const;
        void clearDepth() const;
        void clear(const Color& color = Color(0, 0, 0, 255), LayerType type = LayerType::Both) const;
        void draw(const Drawable& object, const Context& context);

        virtual const Math::Vec2u& getSize() const = 0;
    };
}
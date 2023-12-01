#pragma once

#include "../Util/Vector.hpp"

#include "Surface.hpp"
#include "Texture.hpp"

namespace S2D::Graphics
{
    struct DrawTexture : public Surface
    {
        using Handle = uint32_t;
        
        DrawTexture(const DrawTexture&) = delete;
        
        DrawTexture() = default;
        DrawTexture(const Math::Vec2u& size);
        DrawTexture(DrawTexture&& dt);
        ~DrawTexture();

        void bind() const override;

        [[nodiscard]]
        bool create(const Math::Vec2u& size);

        const Math::Vec2u& getSize() const;
        const Texture* texture() const;

    private:
        Handle handle, depth_stencil;

        Math::Vec2u _size;
        Texture _texture;
    };
}
#pragma once

#include "../Def.hpp"
#include "../Util/Vector.hpp"

namespace S2D::Graphics
{
    struct Texture
    {
        friend class DrawTexture;

        using Handle = uint32_t;

        enum class Format
        {
            Red, RGB, RGBA
        };

        Texture(const Texture&) = delete;
        
        Texture() = default;
        Texture(Texture&&);
        ~Texture();

        void bind() const;
        void unbind() const;

        [[nodiscard]]
        bool fromFile(const std::filesystem::path& path);

        [[nodiscard]]
        bool fromMemory(const Math::Vec2u& size, const uint8_t* data, Format format);

        [[nodiscard]]
        bool fromEmpty(const Math::Vec2u& size, Format _format);

    private:
        Handle handle;
    };

}
#pragma once

#include "../Def.hpp"

namespace S2D::Graphics
{
    struct Texture
    {
        using Handle = uint32_t;

        Texture(const Texture&) = delete;
        
        Texture() = default;
        Texture(Texture&&);
        ~Texture();

        void bind() const;
        void unbind() const;

        [[nodiscard]]
        bool fromFile(const std::filesystem::path& path);

    private:
        Handle handle;
    };

}
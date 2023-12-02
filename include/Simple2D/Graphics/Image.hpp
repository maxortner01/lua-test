#pragma once

#include "../Def.hpp"

#include "../Util/Color.hpp"
#include "../Util/Vector.hpp"

namespace S2D::Graphics
{
    struct Image
    {
        using DataType = std::unique_ptr<uint8_t, void(void*)>;

        Image() = default;
        Image(const Image&);
        Image(Image&&);
        ~Image();

        [[nodiscard]]
        bool fromFile(const std::filesystem::path& filepath);

        Graphics::Color read(const Math::Vec2u& position) const;

        const Math::Vec2u& getSize() const { return size; }

    private:
        Math::Vec2u size;
        Graphics::Color* data;
    }; 
}
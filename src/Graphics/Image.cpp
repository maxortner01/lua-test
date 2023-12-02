#include <Simple2D/Graphics/Image.hpp>

#include "stb_image.h"

namespace S2D::Graphics
{

Image::Image(const Image& image) :
    size( image.size ),
    data( image.data ? [&]()
    {
        using Type = Graphics::Color;
        const auto count = image.getSize().x * image.getSize().y;
        auto* ptr = std::calloc(count, sizeof(Type));
        std::memcpy(ptr, image.data, count * sizeof(Type));
        return reinterpret_cast<Type*>(ptr);
    }() : nullptr )
{

}

Image::Image(Image&& image) :
    size( image.size ),
    data( image.data )
{
    image.data = nullptr;
}

Image::~Image()
{
    if (data) std::free(data);
    data = nullptr;
}

bool Image::fromFile(const std::filesystem::path& filepath)
{
    using namespace Graphics;

    int width, height, channels;
    uint8_t* image_data = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!image_data) return false;

    data = (Color*)std::calloc(width * height, sizeof(Color));

    for (uint32_t y = 0; y < height; y++)
        for (uint32_t x = 0; x < height; x++)
        {
            const auto index = y * width + x;
            auto& color = data[index];

            color.r = image_data[index * 4 + 0] / 255.f;
            color.g = image_data[index * 4 + 1] / 255.f;
            color.b = image_data[index * 4 + 2] / 255.f;
            color.a = image_data[index * 4 + 3] / 255.f;
        }

    size = { (uint32_t)width, (uint32_t)height };
    stbi_image_free(image_data);
    return true;
}

Graphics::Color Image::read(const Math::Vec2u& position) const
{
    S2D_ASSERT(position.x < getSize().x && position.y < getSize().y, "Image index out of range");
    return data[position.y * getSize().x + position.x];
}

}
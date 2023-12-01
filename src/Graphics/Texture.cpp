#include <Simple2D/Graphics/Texture.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GL/glew.h>

namespace S2D::Graphics
{

Texture::Texture(Texture&& tex) :
    handle(tex.handle)
{
    tex.handle = 0;
}

Texture::~Texture()
{
    if (handle) glDeleteTextures(1, &handle);
    handle = 0;
}

void Texture::bind() const
{
    S2D_ASSERT(handle, "Attempting to bind a corrupted texture");
    glBindTexture(GL_TEXTURE_2D, handle);
}

void Texture::unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool
Texture::fromFile(
    const std::filesystem::path& filepath)
{
    int32_t width, height, channels;
    uint8_t* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    if (!data) return false;
    //S2D_ASSERT_ARGS(data, "Error loading file \"%s\"", filepath.filename().c_str());

    const auto format = [&]()
    {
        switch (channels)
        {
        case STBI_rgb: return GL_RGB;
        case STBI_rgb_alpha: return GL_RGBA;
        default: return GL_NONE;
        }
    }();

    glGenTextures(1, &handle);
    
    glActiveTexture(GL_TEXTURE0);
    bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    unbind();

    stbi_image_free(data);

    return true;
}

}

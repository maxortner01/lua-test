#include <Simple2D/Graphics/DrawTexture.hpp>

#include <GL/glew.h>

namespace S2D::Graphics
{

DrawTexture::DrawTexture(const Math::Vec2u& size)
{
    S2D_ASSERT(create(size), "Error creating DrawTexture");
}

DrawTexture::DrawTexture(DrawTexture&& dt) :
    handle(dt.handle)
{
    dt.handle = 0;
}

DrawTexture::~DrawTexture()
{
    if (handle) glDeleteFramebuffers(1, &handle);
    handle = 0;
}

void DrawTexture::bind() const
{
    S2D_ASSERT(handle, "Attempting to bind null framebuffer");
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    S2D_ASSERT(depth_stencil, "Attempting to bind null depth and stencil buffer");
    glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil);
    glViewport(0, 0, _size.x, _size.y);
}

bool DrawTexture::create(const Math::Vec2u& size)
{
    glGenFramebuffers(1, &handle);
    glGenRenderbuffers(1, &depth_stencil);
    
    bind();
    S2D_ASSERT(_texture.fromEmpty(size, Texture::Format::RGBA), "Texture failed to create");

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture.handle, 0);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_stencil);

    S2D_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is not complete");
    
    _size = size;

    unbind();

    return true;
}

const Math::Vec2u& DrawTexture::getSize() const
{
    return _size;
}

const Texture* DrawTexture::texture() const
{
    return &_texture;
}

}
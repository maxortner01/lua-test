#include <Simple2D/Graphics/Surface.hpp>
#include <Simple2D/Graphics/Context.hpp>
#include <Simple2D/Graphics/Drawable.hpp>

#include <GL/glew.h>

namespace S2D::Graphics
{
    void Surface::unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    void Surface::draw(const Drawable& object, const Context& context)
    {
        bind();

        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

        if (context.program) context.program->use();

        for (int32_t i = 0; i < context.textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            context.textures[i]->bind();
            // It's possible we can make the Program const because it may just automatically
            // deduce what texture in the shader based off the amount of sample2D uniforms are declared
            // so, there could be no point in this line
            if (context.program) context.program->setUniform("texture" + std::to_string(i), i);
        }

        object.draw(this);

        for (int32_t i = 0; i < context.textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        unbind();
    }

    void Surface::clear(const Color& color) const
    {
        bind();
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        unbind();
    }
}
#include <Simple2D/Graphics/Surface.hpp>
#include <Simple2D/Graphics/Context.hpp>
#include <Simple2D/Graphics/Drawable.hpp>

#include <GL/glew.h>

namespace S2D::Graphics
{
    void Surface::draw(const Drawable& object, const Context& context)
    {
        bind();

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
    }
}
#include <Simple2D/Graphics/DrawWindow.hpp>

#include <Simple2D/Util/Error.hpp>

#include <GL/glew.h>
#include <SDL3/SDL.h>

namespace S2D::Graphics
{
    DrawWindow::DrawWindow(
        const Math::Vec2u& size, 
        const std::string& title) :
            Window(size, title)
    {
        auto& logger = Log::Logger::instance("graphics");
        S2D_ASSERT(window, "Window not initialized");

        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
        context = SDL_GL_CreateContext( reinterpret_cast<SDL_Window*>(window) );
        S2D_ASSERT_ARGS(context, "Error creating GL context: \"%s\"", SDL_GetError());

        glewExperimental = GL_TRUE;
        const auto init = glewInit();
        S2D_ASSERT_ARGS(init == GLEW_OK, "Error loading glew: \"%s\"", glewGetErrorString( init ));

        // Vsync (make option)
        if (SDL_GL_SetSwapInterval( -1 ) < 0) logger->warn("Error initializing vsync: \"{}\"", SDL_GetError());
    }

    DrawWindow::~DrawWindow()
    {
        if (context)
            SDL_GL_DeleteContext(reinterpret_cast<SDL_GLContext*>(context));
        context = nullptr;
    }   

    void DrawWindow::clear(const Color& color) const
    {
        const auto norm = color.normalized();
        glClearColor(norm.x, norm.y, norm.z, norm.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void DrawWindow::display() const
    {
        SDL_GL_SwapWindow(reinterpret_cast<SDL_Window*>(window));
    }
}

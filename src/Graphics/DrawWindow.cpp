#include <Simple2D/Graphics/DrawWindow.hpp>
#include <Simple2D/Graphics/DrawTexture.hpp>

#include <Simple2D/Util/Error.hpp>

#include <GL/glew.h>
#include <SDL3/SDL.h>

#ifdef USE_IMGUI
#   include <imgui.h>
#   include <backends/imgui_impl_opengl3.h>
#   include <backends/imgui_impl_sdl3.h>
#endif

namespace S2D::Graphics
{
    DrawWindow::DrawWindow(
        const Math::Vec2u& size, 
        const std::string& title) :
            Window(size, title),
            _size(size)
    {
        auto& logger = Log::Logger::instance("graphics");
        S2D_ASSERT(window, "Window not initialized");

        SDL_GL_SetAttribute( SDL_GL_RED_SIZE,   8  );
        SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8  );
        SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE,  8  );
        SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8  );
        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
        context = SDL_GL_CreateContext( reinterpret_cast<SDL_Window*>(window) );
        S2D_ASSERT_ARGS(context, "Error creating GL context: \"%s\"", SDL_GetError());

        // Vsync (make option)
        if (SDL_GL_SetSwapInterval( 1 ) < 0) logger->warn("Error initializing vsync: \"{}\"", SDL_GetError());

        glewExperimental = GL_TRUE;
        const auto init = glewInit();
        S2D_ASSERT_ARGS(init == GLEW_OK, "Error loading glew: \"%s\"", glewGetErrorString( init ));

#   ifdef USE_IMGUI
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

        ImGui_ImplSDL3_InitForOpenGL(reinterpret_cast<SDL_Window*>(this->window), context);
        ImGui_ImplOpenGL3_Init();
#   endif
    }

    DrawWindow::~DrawWindow()
    {
#   ifdef USE_IMGUI
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
#   endif
        if (context)
            SDL_GL_DeleteContext(reinterpret_cast<SDL_GLContext*>(context));
        context = nullptr;
    }   

    const Math::Vec2u& DrawWindow::getSize() const
    {
        return _size;
    }

    bool DrawWindow::pollEvent(Event& event) const
    {
        SDL_Event e;
        if (SDL_PollEvent(&e))
        {
#       ifdef USE_IMGUI
            ImGui_ImplSDL3_ProcessEvent(&e);
#       endif
            standardEvents(reinterpret_cast<void*>(&e), event);
            return true;
        }
#   ifdef USE_IMGUI
        else 
        {
            // For now, if this is the last event, start the ImGui context
            ImGui_ImplSDL3_NewFrame();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();
            return false;
        }
#   else
        else return false;
#   endif
    }

    void DrawWindow::display() const
    {
#   ifdef USE_IMGUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#   endif
        SDL_GL_SwapWindow(reinterpret_cast<SDL_Window*>(window));
    }

    void DrawWindow::bind() const
    {
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, getSize().x, getSize().y);
    }
}

#include <Simple2D/Engine/Core.hpp>
#include <Simple2D/Engine/LuaLib/Surface.hpp>

#include <Simple2D/Util/Transform.hpp>

#include <Simple2D/Log/Log.hpp>

#ifdef USE_IMGUI
#   include <imgui.h>
#   include <backends/imgui_impl_opengl3.h>
#   include <backends/imgui_impl_sdl3.h>
#endif

namespace S2D::Engine
{

void Core::render(Scene* scene)
{   
    auto& log = Log::Logger::instance("engine");

    Graphics::DrawTexture* current_target = &fbo;
    const auto& targets  = scene->renderpass->targets;
    const auto& commands = scene->renderpass->commands;
    
    auto& renderer = scene->renderer;
    S2D_ASSERT(renderer, "Renderer is corrupted");

    const auto camera = scene->world.filter<const Camera>().first();

    #define GET_PARAMS(cmd) auto* params = (CommandParameters<cmd>*)command.second.get();

    for (const auto& command : commands)
    {
        switch (command.first)
        {
        case Command::BindSurface:
        {
            GET_PARAMS(Command::BindSurface);
            if (targets.count(params->name)) current_target = targets.at(params->name).get();
            else log->error("Requesting target that doesn't exist");

            break;
        }
        case Command::Clear:
        {
            if (!current_target)
            {
                log->error("Attempting to clear null surface");
                break;
            }

            GET_PARAMS(Command::Clear);
            current_target->clear(params->color, params->layer);

            break;
        }
        case Command::RenderEntities:
        {
            if (!current_target)
            {
                log->error("Attempting to render entities to null surface");
                break;
            }

            GET_PARAMS(Command::RenderEntities);

            auto camera_to_use = (params->camera_name.size() ? scene->world.lookup(params->camera_name.c_str()) : camera);
            if (!camera_to_use)
            {
                log->error("Error using camera while rendering entities");
                break;
            }

            renderer->render(camera_to_use, *current_target);

            break;
        }
        case Command::RenderEntity:
        {
            GET_PARAMS(Command::RenderEntity);

            if (!current_target)
            {
                log->error("Attempting to render entity \"{}\" to null surface", params->entity_name);
                break;
            }

            auto camera_to_use = (params->camera_name.size() ? scene->world.lookup(params->camera_name.c_str()) : camera);
            if (!camera_to_use)
            {
                log->error("Error using camera while rendering entity \"{}\"", params->entity_name);
                break;
            }

            Graphics::Program* program = nullptr;
            if (params->shader_name.size())
            {
                const auto res = scene->resources.getResource<Graphics::Program>(params->shader_name);
                if (!res) 
                {
                    log->error("Error loading shader {}", params->shader_name);
                    break;
                }

                program = res.value();
            }

            renderer->render(camera_to_use, params->entity_name.c_str(), *current_target, program);

            break;
        }
        case Command::BlitSurface:
        {
            if (!current_target)
            {
                log->error("Attempting to blit null surface to window");
                break;
            }

            GET_PARAMS(Command::BlitSurface);

            /* Need to be able to draw sprite */
            Math::Transform model;
            model.scale({ 
                (float)params->size.x / (float)current_target->getSize().x * 2.f, 
                (float)params->size.y / (float)current_target->getSize().y * 2.f, 
                1.f 
            });

            // this might be wrong
            model.translate({
                params->position.x / (2.f * window.getSize().x),
                params->position.y / (2.f * window.getSize().y),
                0.f
            });

            Renderer::QuadInfo info;
            info.texture = current_target->texture();
            info.depth_test = false;

            renderer->renderQuad(model, fbo, info);

            break;
        }
        case Command::RenderUI:
        {
            if (!current_target)
            {
                log->error("Attempting to render UI on null surface");
                break;
            }
            
            GET_PARAMS(Command::RenderUI);
            auto& runtime = params->runtime;

            // We pass a surface to the runtime which contains the current scene
            // this way the function can access the scene and its resources
            // We also pass a pointer to the current surface
            auto surface = Surface().asTable();
            surface.set("scene",   (void*)scene);
            surface.set("surface", (void*)current_target);
            
            const auto res = runtime->runFunction<>("RenderUI", surface);
            if (!res && res.error().code() != Lua::Runtime::ErrorCode::NotFunction)
                log->error("Error rendering UI ({}): {}", (int)res.error().code(), res.error().message());

            break;
        }
        case Command::RenderFunction:
        {
            if (!current_target)
            {
                log->error("Attempting to run a function on null surface");
                break;
            }

            GET_PARAMS(Command::RenderFunction);

            params->function(scene, *current_target);

            break;
        }
        default: break;
        }
    }

    // After all, we render the fbo to the screen
    Math::Transform model;
    model.scale({ 
        2.f, 
        2.f, 
        1.f 
    });

    Renderer::QuadInfo info;
    info.texture = fbo.texture();
    info.depth_test = false;

    renderer->renderQuad(model, window, info);

#ifdef USE_IMGUI
    //ImGui::SetNextWindowPos(ImVec2(0, 0));
    //ImGui::SetNextWindowSize(ImVec2(window.getSize().x, window.getSize().y));

    ImGui::Begin("Hello");

    ImGui::Text("This is some useful text");

    ImGui::End();

    /*
    ImGui::Begin("GameWindow");
    {
        ImGui::BeginChild("GameRender");

        ImVec2 wsize = ImGui::GetWindowSize();
        ImGui::Image((ImTextureID)fbo.texture()->id(), wsize, ImVec2(0, 1), ImVec2(1, 0));

        ImGui::EndChild();
    }
    ImGui::End();*/
#else
    // After all, we render the fbo to the screen
    Math::Transform model;
    model.scale({ 
        2.f, 
        2.f, 
        1.f 
    });

    Renderer::QuadInfo info;
    info.texture = fbo.texture();
    info.depth_test = false;

    renderer->renderQuad(model, window, info);
#endif
}

}

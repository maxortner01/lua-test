#include <Simple2D/Engine/Core.hpp>
#include <Simple2D/Engine/LuaLib/Surface.hpp>
#include <Simple2D/Engine/LuaLib/Time.hpp>

#include <Simple2D/Util/Transform.hpp>

#include <Simple2D/Log/Log.hpp>

#ifdef USE_IMGUI
#   include <sstream>
#   include <imgui.h>
#   include <imgui_internal.h>
#   include <backends/imgui_impl_opengl3.h>
#   include <backends/imgui_impl_sdl3.h>
#endif

namespace S2D::Engine
{

#ifdef USE_IMGUI

template<typename Data>
void render_data(Data* data) {}

template<>
void render_data<Transform>(Transform* data)
{
    const auto table = Component<Name::Transform>::getTable(*data);
    if (ImGui::TreeNode(*Name::Transform))
    {
        ImGui::SliderFloat3("position", (float*)&data->position, -1000.f, 1000.f, "%.2f");
        ImGui::SliderFloat("scale", &data->scale, 0.f, 10.f);
        ImGui::SliderFloat("rotation", &data->rotation, -180.f, 180.f, "%.1f");
        
        ImGui::TreePop();
    }
}

void render_resource_pane(Scene* scene)
{
    // Fonts
    const auto fonts = scene->resources.getResourceMap<Graphics::Font>();
    if (fonts)
    {
        if (ImGui::TreeNode("Fonts"))
        {
            for (const auto& p : *fonts.value())
                ImGui::Text("%s", p.first.c_str());
            ImGui::TreePop();
        }
    }
    else
    {
        ImGui::Text("No fonts loaded.");
    }

    // Textures, Drawtextures, images
    const auto textures = scene->resources.getResourceMap<Graphics::Texture>();
    const auto draw_textures = scene->resources.getResourceMap<Graphics::DrawTexture>();
    const auto images = scene->resources.getResourceMap<Graphics::Image>();

    bool started = false;
    bool texture_node = false;
    bool some_images = false;
    if (textures)
    {
        if (!started) 
        {
            started = true;
            texture_node = ImGui::TreeNode("Textures");
        }
        some_images = true;

        if (texture_node)
        {
            const auto& t = textures.value();
            for (const auto& p : *t)
            {
                const Graphics::Texture* tex = reinterpret_cast<const Graphics::Texture*>(p.second.get());
                if (ImGui::TreeNode(p.first.c_str()))
                {
                    ImGui::Text("Size: (%u, %u)", tex->getSize().x, tex->getSize().y);

                    ImVec2 wsize = ImGui::GetItemRectSize();
                    const auto aspect = (float)tex->getSize().x / (float)tex->getSize().y;

                    ImGui::Image((ImTextureID)tex->id(), ImVec2(wsize.x, wsize.x / aspect), ImVec2(0, 1), ImVec2(1, 0));
                    
                    ImGui::TreePop();
                }
            }
        }
    }

    if (draw_textures)
    {
        if (!started) 
        {
            started = true;
            texture_node = ImGui::TreeNode("Textures");
        }
        some_images = true;

        if (texture_node)
        {
            const auto& t = textures.value();
            for (const auto& p : *t)
                ImGui::Text("%s", p.first.c_str());
        }
    }

    if (images)
    {
        if (!started) 
        {
            started = true;
            texture_node = ImGui::TreeNode("Textures");
        }
        some_images = true;

        if (texture_node)
        {
            const auto& t = textures.value();
            for (const auto& p : *t)
                ImGui::Text("%s", p.first.c_str());
        }
    }

    if (texture_node) ImGui::TreePop();

    if (!some_images)
    {
        ImGui::Text("No textures loaded.");
    }

    // Programs
    const auto programs = scene->resources.getResourceMap<Graphics::Program>();

    if (programs)
    {
        if (ImGui::TreeNode("Programs"))
        {
            ImGui::TreePop();
        }
    }
    else
    {
        ImGui::Text("No shaders loaded.");
    }
}

void render_entities_pane(Scene* scene)
{
    auto& world = scene->world;
        
    uint32_t count = 1;
    world.children([&](flecs::entity e)
    {
        bool has = false;
        Util::CompileTime::static_for<(int)Name::Count>([&](auto n)
        {
            if (has) return;
            const std::size_t i = n;
            constexpr Name name = (Name)i;
            using Type = Component<name>;
            if (e.has<ComponentData<name>>())
            {
                has = true;
            }
        });

        if (!has) return;

        const auto name = ( 
            e.name().length() ? 
            std::string(e.name()) : 
            (std::stringstream() << "[Unnamed Entity " << count++ << "]").str().c_str() 
        );

        if (ImGui::TreeNode(name.c_str()))
        {
            Util::CompileTime::static_for<(int)Name::Count>([&](auto n)
            {
                const std::size_t i = n;
                constexpr Name name = (Name)i;
                using Type = Component<name>;
                if (e.has<ComponentData<name>>())
                {
                    render_data(e.get_mut<ComponentData<name>>());
                }
            });
            ImGui::TreePop();
        }
    });
}

void Core::render_imgui(Scene* scene)
{
    const std::function<void(const Lua::Table&)> push_table = 
        [&](const Lua::Table& table)
        {
            for (const auto& p : table.getMap())
            {
                using namespace Lua::CompileTime;
                /**/ if (p.second.type == TypeMap<Lua::String>::LuaType)
                {
                    const Lua::String* value = reinterpret_cast<Lua::String*>(p.second.data.get());
                    ImGui::Text("%s: %s", p.first.c_str(), value->c_str());
                }
                else if (p.second.type == TypeMap<Lua::Number>::LuaType)
                {
                    const Lua::Number* value = reinterpret_cast<Lua::Number*>(p.second.data.get());
                    ImGui::Text("%s: %f", p.first.c_str(), *value);
                }
                else if (p.second.type == TypeMap<Lua::Boolean>::LuaType)
                {
                    const Lua::Boolean* value = reinterpret_cast<Lua::Boolean*>(p.second.data.get());
                    ImGui::Text("%s: %s", p.first.c_str(), ( *value ? "True" : "False" ));
                }
                else if (p.second.type == TypeMap<Lua::Table>::LuaType)
                {
                    const Lua::Table* value = reinterpret_cast<Lua::Table*>(p.second.data.get());
                    if (ImGui::TreeNode(p.first.c_str()))
                    {
                        push_table(*value);
                        ImGui::TreePop();
                    }
                }
            }
        };

    std::string fps = (std::stringstream() << std::fixed << std::setprecision(2) << 1.0 / Time::dt).str();
    //ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    bool begin = true;
    if (!ImGui::Begin("Main", &begin, window_flags)) return;

    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MainSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

    static bool first_time = true;
    if (first_time)
    {
        first_time = false;
        // Clear out existing layout
        ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
        ImGui::DockBuilderAddNode(dockspace_id); // Add empty node
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

        ImGuiID dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
        ImGuiID dock_id_prop = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.25f, NULL, &dock_main_id);
        ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);

        ImGui::DockBuilderDockWindow("Top", dock_main_id);
        ImGui::DockBuilderDockWindow("Log", dock_id_bottom);
        ImGui::DockBuilderDockWindow("Properties", dock_id_prop);
        ImGui::DockBuilderDockWindow("Mesh", dock_id_prop);
        ImGui::DockBuilderDockWindow("Extra", dock_id_prop);
        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::Begin("Log");
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,0,0,255));
    ImGui::Text("Error");
    ImGui::PopStyleColor();

    for (uint32_t i = 0; i < 10; i++)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,255,0,255));
        ImGui::Text("Warning");
        ImGui::PopStyleColor();
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    ImGui::End();
    
    ImGui::Begin("Properties");
    ImGui::Text("Properties");
    ImGui::End();

    ImGui::Begin("Mesh");
    ImGui::Text("Mesh");
    ImGui::End();

    ImGui::Begin("Top");
    ImGui::Text("No scene loaded");
    ImGui::End();


/*
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }*/

    /*
    ImGui::Begin("Debug");
    ImGui::Text("FPS: %s", fps.c_str());

    //ImGui::DockSpace
    
    if (ImGui::CollapsingHeader("Entities"))
    {
        if (scene)
            render_entities_pane(scene);
        else
            ImGui::Text("No scene loaded.");
    }

    if (ImGui::CollapsingHeader("Resources"))
    {
        if (scene)
            render_resource_pane(scene);
        else
            ImGui::Text("No scene loaded");
    }
    ImGui::End();
    */

    ImGui::End();
}

#endif

void Core::render(Scene* scene)
{   
    auto log = Log::Logger::instance("engine");

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
                (float)fbo.getSize().x,
                (float)fbo.getSize().y,
                1.f
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
        (float)window.getSize().x,
        (float)window.getSize().y,
        1.f
    });

    Renderer::QuadInfo info;
    info.texture = fbo.texture();
    info.depth_test = false;

    renderer->renderQuad(model, window, info);

#ifdef USE_IMGUI
    render_imgui(scene);

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

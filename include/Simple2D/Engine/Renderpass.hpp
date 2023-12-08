#pragma once

#include "../Util.hpp"
#include "../Lua.hpp"

#include <Simple2D/Graphics.hpp>

#include <functional>
#include <vector>
#include <memory>

namespace S2D::Engine
{
    struct Scene;

    enum class Command
    {
        Clear,
        BindSurface,
        RenderEntities,
        RenderEntity,
        RenderUI,
        BlitSurface,
        RenderFunction,
        Count
    };

    enum class Resource
    {
        Surface,
        Count
    };

    template<Command C>
    struct CommandParameters;

    template<>
    struct CommandParameters<Command::Clear>
    {
        Graphics::Color color;
        Graphics::LayerType layer;
    };

    template<>
    struct CommandParameters<Command::BindSurface>
    {
        std::string name;
    };

    template<>
    struct CommandParameters<Command::RenderEntities>
    {  
        std::string camera_name;
    };

    template<>
    struct CommandParameters<Command::RenderEntity>
    {
        std::string entity_name;
        std::string camera_name;
        std::string shader_name;
    };

    template<>
    struct CommandParameters<Command::RenderUI>
    {
        std::unique_ptr<Lua::Runtime> runtime;
    };

    // Blits the currently bound surface to window
    template<>
    struct CommandParameters<Command::BlitSurface>
    {
        Math::Vec2f position, size;
    };

    template<>
    struct CommandParameters<Command::RenderFunction>
    {
        std::function<void(Scene*, Graphics::Surface&)> function;
    };

    template<Resource R>
    struct ResourceParameters;

    template<>
    struct ResourceParameters<Resource::Surface>
    {
        std::string name;
        Math::Vec2u size;
    };

    struct Renderpass
    {
        std::unordered_map<std::string, std::unique_ptr<Graphics::DrawTexture>> targets;
        std::vector<std::pair<Command, std::shared_ptr<void>>> commands;
    };

    struct RenderpassBuilder
    {
    private:
        using Pointer = std::unique_ptr<void, void(*)(void*)>;

    public:
        template<Command C>
        RenderpassBuilder& command(CommandParameters<C>&& parameters)
        { 
            using CP = CommandParameters<C>;
            commands.push_back(std::pair(
                C,
                std::shared_ptr<void>(
                    new CP(std::move(parameters)),
                    [](void* ptr) { delete reinterpret_cast<CP*>(ptr); }
                )
            ));
            return *this;
        }

        template<Resource R>
        void resource(ResourceParameters<R>&& parameters)
        {
            using RP = ResourceParameters<R>;
            resources.push_back(std::pair(
                R,
                Pointer(
                    new RP(parameters),
                    [](void* ptr) { delete reinterpret_cast<RP*>(ptr); }
                )
            ));
        }

        std::unique_ptr<Renderpass> build() const
        {
            using namespace Util::CompileTime;

            auto renderpass = std::make_unique<Renderpass>();

            for (const auto& res : resources)
            {
                static_for<(int)Resource::Count>([&](auto n)
                {
                    const std::size_t i = n;
                    if ((Resource)i != res.first) return;

                    using RP = ResourceParameters<(Resource)i>;
                    const auto& params = *(RP*)res.second.get();

                    // Switch this out for a method that takes the type and constructs a target
                    auto* surface = new Graphics::DrawTexture();
                    S2D_ASSERT(surface->create(params.size), "Surface failed to create.");
                    renderpass->targets.insert(std::pair(
                        params.name,
                        std::unique_ptr<Graphics::DrawTexture>(surface)
                    ));
                });
            }

            renderpass->commands.reserve(commands.size());
            std::copy(commands.begin(), commands.end(), std::back_inserter(renderpass->commands));

            return renderpass;
        }

    private:
        std::vector<std::pair<Resource, Pointer>> resources;
        std::vector<std::pair<Command,  std::shared_ptr<void>>> commands;
    };

}

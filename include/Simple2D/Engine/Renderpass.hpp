#pragma once

#include "../Util.hpp"
#include "../Lua.hpp"

#include <SFML/Graphics.hpp>

#include <vector>
#include <memory>

namespace S2D::Engine
{
    enum class Command
    {
        Clear,
        BindSurface,
        RenderEntities,
        RenderUI,
        BlitSurface,
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
        sf::Color color;
    };

    template<>
    struct CommandParameters<Command::BindSurface>
    {
        std::string name;
    };

    template<>
    struct CommandParameters<Command::RenderEntities>
    {  
        int _temp;
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
        sf::Vector2f position, size;
    };

    template<Resource R>
    struct ResourceParameters;

    template<>
    struct ResourceParameters<Resource::Surface>
    {
        std::string name;
        sf::Vector2u size;
    };

    struct Renderpass
    {
        std::unordered_map<std::string, std::unique_ptr<sf::RenderTexture>> targets;
        std::vector<std::pair<Command, std::shared_ptr<void>>> commands;
    };

    struct RenderpassBuilder
    {
    private:
        using Pointer = std::unique_ptr<void, void(*)(void*)>;

    public:
        template<Command C>
        void command(CommandParameters<C>&& parameters)
        { 
            using CP = CommandParameters<C>;
            commands.push_back(std::pair(
                C,
                std::shared_ptr<void>(
                    new CP(std::move(parameters)),
                    [](void* ptr) { delete reinterpret_cast<CP*>(ptr); }
                )
            ));
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
                    auto* surface = new sf::RenderTexture();
                    S2D_ASSERT(surface->create(params.size), "Surface failed to create.");
                    renderpass->targets.insert(std::pair(
                        params.name,
                        std::unique_ptr<sf::RenderTexture>(surface)
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
#include <Simple2D/Lua.hpp>
#include <Simple2D/Engine.hpp>

#include <sstream>
#include <iostream>
#include <chrono>
#include <numeric>

#include <flecs.h>

using namespace S2D;

#ifndef SOURCE_DIR
#define SOURCE_DIR "."
#endif

struct MainScene : Engine::LuaScene
{
    MainScene(const std::string& filename) :
        LuaScene(filename)
    {   }

    void constructPass(Engine::RenderpassBuilder& builder) override
    {
        using namespace S2D::Engine;

        Log::Logger::instance("game")->info("Constructing pass");

        // Should make command into a template and the argument be a 
        // corresponding data structure to specify information
        // For example, clear gets a color, newsurface gets dimensions, etc.
        // We store it in a vector of std::pair<Command, std::unique_ptr<void>>, where
        // the .second is the corresponding data

        builder.resource<Resource::Surface>({ "main", { 1280, 720 } });

        builder.command<Command::BindSurface>({ "main" });
        builder.command<Command::Clear>({ { 0, 0, 0, 255 } });
        builder.command<Command::RenderEntities>({});
        builder.command<Command::RenderUI>({ loadRuntime(SOURCE_DIR "/scripts/ui.lua", world) });
        builder.command<Command::BlitSurface>({ { 0.f, 0.f }, { 1280.f, 720.f } });
    }
};

struct App : Engine::Application
{
    App(const sf::Vector2u& size, const std::string& name) : Application(size)
    {   
        this->name = name;
    }

    void start(Engine::Core& core) override
    {
        core.emplaceScene<MainScene>(SOURCE_DIR "/scripts/test.lua");
    }
};

Engine::Application* getApplication()
{
    /* Extract window size info from a config script */
    auto window = [&]()
    {
        Lua::Runtime runtime(SOURCE_DIR "/scripts/config.lua");
        auto res = runtime.getGlobal<Lua::Table>("Window");
        S2D_ASSERT(res, "Error loading the window information");
        return res.value();
    }();

    const auto size = [&]()
    {
        const auto& size = window.get<Lua::Table>("size");
        return sf::Vector2u({
            (unsigned int)size.get<Lua::Number>("width"),
            (unsigned int)size.get<Lua::Number>("height")
        });
    }();

    const auto& name = window.get<Lua::String>("title");
    Engine::Script::SourceDir = std::string(SOURCE_DIR);

    return new App(size, name);
}
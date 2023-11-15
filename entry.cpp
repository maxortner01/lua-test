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

struct MainScene : Engine::Scene
{
    void start() override
    {
        using namespace Engine;

        resources.loadResource<sf::Font>   ("arial", SOURCE_DIR "/fonts/arial.ttf"  );
        resources.loadResource<sf::Texture>("main",  SOURCE_DIR "/textures/test.png");

        world.entity()
            .set(ComponentData<Name::Tilemap>{ })
            .set(loadScript(SOURCE_DIR "/scripts/tilemap.lua", world));

        world.entity()
            .set(loadScript(SOURCE_DIR "/scripts/fps.lua", world))
            .set(ComponentData<Name::Transform>{ .position = { 100.f, 100.f, 0.f }, .rotation = 0.f })
            .set(ComponentData<Name::Text>{ .string = "Hello", .font = "arial", .character_size = 16 });

        world.entity()
            .set(ComponentData<Name::Transform>{ .position = { 500.f, 100.f, -10.f }, .rotation = 0.f })
            .set(ComponentData<Name::Sprite>{ .size = sf::Vector2u(32, 32), .texture = "main" });
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
        core.emplaceScene<MainScene>();
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

    return new App(size, name);
}
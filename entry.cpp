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

        std::cout << "in\n";
        assert(resources.loadResource<sf::Font>("arial", SOURCE_DIR "/fonts/arial.ttf"));
        assert(resources.loadResource<sf::Texture>("main", SOURCE_DIR "/textures/test.png"));
        std::cout << "out\n";

        world.entity()
            .set(loadScript(SOURCE_DIR "/scripts/fps.lua", world))
            .set(ComponentData<Name::Transform>{ .position = { 100.f, 100.f, 0.f }, .rotation = 0.f })
            .set(ComponentData<Name::Text>{ .string = "Hello", .font = "arial", .character_size = 16 });

        world.entity()
            .set(ComponentData<Name::Transform>{ .position = { 500.f, 100.f, -10.f }, .rotation = 0.f })
            .set(ComponentData<Name::Sprite>{ .size = sf::Vector2u(32, 32), .texture = "main" });
    }
};

int main()
{
    auto& display = Engine::Display::get();
    display.size = sf::Vector2u({ 1280U, 720U });
    display.name = "Test";

    auto& core = Engine::Core::get();
    core.emplaceScene<MainScene>();
    core.run();

    Engine::Core::destroy();
}

#include <Simple2D/Lua.hpp>
#include <Simple2D/Engine.hpp>

#include <iostream>
#include <chrono>
#include <numeric>

#include <flecs.h>

using namespace S2D;

#ifndef SOURCE_DIR
#define SOURCE_DIR ""
#endif

struct MainScene : Engine::Scene
{
    void start() override
    {
        using namespace Engine;

        world.entity()
            .set(loadScript<Log::Library>(SOURCE_DIR "/scripts/start.lua", world))
            .set(ComponentData<Name::Position>{ .x = 10.f, .y = 10.f });
    }

    void draw(sf::RenderTarget& target) override
    {
        auto& logger = Log::Logger::instance("MainScene");

        world.filter<Engine::ComponentData<Engine::Name::Position>>().each(
            [&](Engine::ComponentData<Engine::Name::Position>& pos)
        {
            logger->info("Position = ({}, {})", pos.x, pos.y);
        });
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
}

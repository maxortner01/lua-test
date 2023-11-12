#include <Simple2D/Lua.hpp>
#include <Simple2D/Log.hpp>

// Before we include the engine header, we have to define two things
// Firstly is COMPONENT_ENUM_NAME which defines the enum that enumerates each component
// type, has a Count member, and has a const char* operator* defined for it as well as 
// COMPONENT_STRUCT_NAME, which is the name of the struct that has a specialization for
// each COMPONENT_ENUM_NAME's members each of which contain a fromTable and toTable static
// method
#include "GameComps.hpp"
#include <Simple2D/Engine.hpp>

#include <iostream>
#include <chrono>
#include <numeric>

#ifndef SOURCE_DIR
#define SOURCE_DIR ""
#endif

int main()
{
    auto& logger = Log::Logger::instance("engine");
    logger->info("Hello");

    auto& world = Util::Singleton<flecs::world>::get();
    auto entity = world.entity()
        .set(Script{.runtime = std::make_unique<Lua::Runtime>([&](){ 
            auto runtime = Lua::Runtime::create<
                Log::Library
            >(SOURCE_DIR "/scripts/new_test.lua");

            Lua::Table globals;
            globals.set("Position",  (Lua::Number)world.component<Component<Name::Position>::Data>().raw_id());
            globals.set("RigidBody", (Lua::Number)world.component<Component<Name::RigidBody>::Data>().raw_id());

            runtime.setGlobal("Component", globals);

            return runtime;
         }())})
        .set(Component<Name::Position>::Data{.x = 10.f, .y = 50.f})
        .set(Component<Name::RigidBody>::Data{ .velocity = { 2.f, 5.f } });
    
    // Execute the Start methods of the scripts
    auto scripts = world.filter<Script>();
    scripts.each([](Script& script)
    {
        script.runtime->runFunction<>("Start");
    });

    // Create Script update system
    world.system<Script>()
        .each([&](flecs::entity e, Script& s)
        {
            Lua::Table ent;
            ent.set("id", e.raw_id());
            ent.set("getComponent", (Lua::Function)Engine::Library::getComponent);
            ent.set("setComponent", (Lua::Function)Engine::Library::setComponent);
            s.runtime->runFunction<>("Update", ent);
        });

    std::vector<double> micros(1000);
    for (uint32_t i = 0; i < 1000; i++)
    {
        auto start = std::chrono::high_resolution_clock::now();

        world.progress();

        auto micro = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
        micros[i] = micro;
    }

    double average = std::accumulate(micros.begin(), micros.end(), 0.0) / micros.size();

    logger->info("Execution time avg. of {} microseconds", average);

    const auto* position = entity.get<Component<Name::Position>::Data>();
    std::cout << "[C++] Position = (" << position->x << ", " << position->y << ")\n";

    return 0;
}

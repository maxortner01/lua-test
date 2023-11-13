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

int main()
{
    using namespace Engine;

    auto& logger = Log::Logger::instance("engine");
    logger->info("Hello");

    auto& world = Util::Singleton<flecs::world>::get();
    auto entity = world.entity()
        .set(Script{.runtime = std::make_unique<Lua::Runtime>([&](){ 
            auto runtime = Lua::Runtime::create<
                Log::Library
            >(SOURCE_DIR "/scripts/new_test.lua");

            Lua::Table globals;
            registerComponents(globals);
            runtime.setGlobal("Component", globals);

            return runtime;
         }())})
        .set(ComponentData<Name::Position>{.x = 10.f, .y = 50.f})
        .set(ComponentData<Name::Rigidbody>{ .velocity = { 2.f, 10.f } });
    
    // Execute the Start methods of the scripts
    auto scripts = world.filter<Script>();
    scripts.each([](flecs::entity e, Script& script)
    {
        auto ent = Engine::Entity().asTable();
        ent.set("id", e.raw_id());

        script.runtime->runFunction<>("Start", ent);
    });

    // Create Script update system
    world.system<Script>()
        .each([&](flecs::entity e, Script& s)
        {   
            auto ent = Engine::Entity().asTable();
            ent.set("id", e.raw_id());

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

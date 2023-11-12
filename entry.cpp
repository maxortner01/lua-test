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
            >(SOURCE_DIR "/scripts/entity.lua");

            Lua::Table globals;
            globals.set<Lua::String>("Position", "Position");
            globals.set<Lua::String>("RigidBody", "RigidBody");

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

    world.progress();

    return 0;
}

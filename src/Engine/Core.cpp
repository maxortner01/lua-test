#include <Simple2D/Engine/Core.hpp>
#include <Simple2D/Engine/Components.hpp>
#include <Simple2D/Engine/Entity.hpp>
#include <Simple2D/Engine/World.hpp>

#include <iostream>

namespace S2D::Engine
{

Scene* Core::getTopScene()
{
    return _scenes.top();
}

void Core::run()
{
    while (window.isOpen() && _scenes.size())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

    get_top_scene:
        if (!_scenes.size()) break;
        auto* top_scene = getTopScene();
        if (!top_scene->running)
        {
            delete top_scene;
            _scenes.pop();
            goto get_top_scene;
        }

        // We execute all the scripts
        // THEN the other registered systems run with world.progress()
        // THEN the draw method is called

        auto& world = top_scene->world;
        auto scripted_entities = world.query<Script>();
        scripted_entities.each([&](flecs::entity e, Script& script)
        {
            if (!e.is_alive() || e.has<Dead>()) return;

            assert(script.runtime);
            if (!script.initialized) { script.runtime->template runFunction<>("Start"); script.initialized = true; }

            // Execute the update function
            auto ent = Engine::Entity().asTable();
            ent.set("id", e.raw_id());
            ent.set("good", true);
            ent.set("world", (uint64_t)world.c_ptr()); // Currently hacky way to store a pointer (must be considered an int64)

            auto _world = Engine::World().asTable();
            _world.set("id", (uint64_t)world.c_ptr());
            _world.set("good", true);

            script.runtime->template runFunction<>("Update", _world, ent);
            //if (values && !std::get<0>(values.value())) top_scene->running = false;

            // Check if it has a collider component and execute the collision function
        });

        if (world.count<Dead>())
        {
            std::vector<flecs::entity> dead_entities;
            dead_entities.reserve(world.count<Dead>());
            world.filter<Dead>().each([&](flecs::entity e, Dead d)
                { dead_entities.push_back(e); }
            );
            for (auto& e : dead_entities) e.destruct();
        }

        window.clear();
        world.progress();
        top_scene->draw(window);
        window.display();
    }
}

Core::Core() :
    window(
        sf::VideoMode(
            Display::get().size
        ),
        Display::get().name
    )
{

}

Core::~Core()
{
    while (_scenes.size()) { delete _scenes.top(); _scenes.pop(); }
}

}

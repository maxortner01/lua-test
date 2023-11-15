#include <Simple2D/Engine.hpp>
#include <Simple2D/Def.hpp>

namespace S2D::Engine
{

Scene* Core::getTopScene()
{
    return _scenes.top();
}

void Core::run()
{
    auto tick = std::chrono::high_resolution_clock::now();

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

            S2D_ASSERT(script.runtime, "Script runtime is null!");
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

        render(top_scene);
        top_scene->draw(window);

        window.display();

        auto now = std::chrono::high_resolution_clock::now();
        Time::dt = std::chrono::duration_cast<std::chrono::microseconds>(now - tick).count() / 1e6;
        tick = now;
    }
}

Core::Core(const Application& app) :
    window(
        sf::VideoMode(app.size),
        app.name
    )
{   }

Core::~Core()
{
    while (_scenes.size()) { delete _scenes.top(); _scenes.pop(); }
}

void Core::render(Scene* scene)
{
    // possibly only create the query once
    auto query = scene->world.query_builder<const ComponentData<Name::Transform>>()
        .order_by<ComponentData<Name::Transform>>(
            [](flecs::entity_t e1, const ComponentData<Name::Transform> *d1, flecs::entity_t e2, const ComponentData<Name::Transform> *d2) 
            {
                return (d1->position.z > d2->position.z) - (d1->position.z < d2->position.z);
            })
        .build();
    
    query.each(
        [&](flecs::entity e, const ComponentData<Name::Transform>& transform)
        {
            /* Render Sprites */
            const auto* sprite = e.get<const ComponentData<Name::Sprite>>();
            if (sprite)
            {
                const sf::Texture* texture = nullptr;
                if (sprite->texture.size())
                    texture = scene->resources.getResource<sf::Texture>(sprite->texture).value();

                sf::RectangleShape rect;
                rect.setSize((sf::Vector2f)sprite->size);
                rect.setOrigin(rect.getSize() / 2.f);
                rect.setFillColor(sf::Color::White);
                rect.setPosition(sf::Vector2f((int)transform.position.x, (int)transform.position.y));
                rect.rotate(sf::degrees(transform.rotation));
                
                if (texture) rect.setTexture(texture);
                
                window.draw(rect);
            }

            /* Render Text */
            const auto* text = e.get<const ComponentData<Name::Text>>();
            if (text)
            {
                auto* font = scene->resources.getResource<sf::Font>(text->font).value();
                S2D_ASSERT(font, "Font is null");
                sf::Text t(*font, text->string);
                t.setPosition(sf::Vector2f(
                    (int)transform.position.x,
                    (int)transform.position.y
                ));
                t.setOrigin(sf::Vector2f(
                    (int)t.getLocalBounds().left,
                    (int)t.getLocalBounds().top
                ));
                t.setCharacterSize((unsigned int)text->character_size);
                t.setFillColor(sf::Color::White);
                window.draw(t);
            }
        }
    );
}

}

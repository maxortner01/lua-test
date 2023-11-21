#include <Simple2D/Engine/Core.hpp>
#include <Simple2D/Engine/Input.hpp>
#include <Simple2D/Engine/Entity.hpp>
#include <Simple2D/Engine/World.hpp>
#include <Simple2D/Engine/Time.hpp>
#include <Simple2D/Engine/Application.hpp>

#include <Simple2D/Log/Log.hpp>

namespace S2D::Engine
{

Scene::Scene() :
    scripts(
        world.query_builder<Script>().build()
    ),
    transforms(
        world.query_builder<const Transform>()
            .order_by<Transform>(
            [](flecs::entity_t e1, const Transform *d1, flecs::entity_t e2, const Transform *d2) 
            {
                return (d1->position.z > d2->position.z) - (d1->position.z < d2->position.z);
            })
            .build()
    ),
    colliders(
        world.query_builder<const Collider, Transform, Rigidbody>().build()
    ),
    dead(
        world.query_builder<Dead>().build()
    ),
    rigidbodies(
        world.query_builder<Transform, Rigidbody>().build()
    )
{

}

Scene* Core::getTopScene()
{
    return _scenes.top();
}

#define ITEM(x) case sf::Keyboard::Key::x: return #x;
std::string getSFMLKey(const sf::Keyboard::Key& key)
{
    switch (key)
    {
    ITEM(W)
    ITEM(A)
    ITEM(S)
    ITEM(D)
    default: return "";
    }
}

std::string getSFMLMouse(const sf::Mouse::Button& button)
{
    switch (button)
    {
    case sf::Mouse::Button::Left:  return *Input::Button::LeftClick;
    case sf::Mouse::Button::Right: return *Input::Button::RightClick;
    default: return "";
    }
}

void Core::run()
{
    auto tick = std::chrono::high_resolution_clock::now();

    while (window.isOpen() && _scenes.size())
    {
        sf::Event event;

        // Change all KeyState::Pressed to KeyState::Down
        // Erase all KeyState::Released
        std::vector<std::string> released;
        for (auto& p : Input::global_state)
        {
            if (p.second == Input::KeyState::Press) p.second = Input::KeyState::Down;
            if (p.second == Input::KeyState::Release) released.push_back(p.first);
        }

        for (const auto& c : released) Input::global_state.erase(c);

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed)
            {
                const auto code = getSFMLKey(event.key.code);
                if (!Input::global_state.count(code)) Input::global_state.insert(std::pair(code, Input::KeyState::Press));
            }
            else if (event.type == sf::Event::KeyReleased)
            {
                const auto code = getSFMLKey(event.key.code);
                if (Input::global_state.count(code)) Input::global_state.at(code) = Input::KeyState::Release;
            }
            else if (event.type == sf::Event::MouseButtonPressed)
            {
                const auto code = getSFMLMouse(event.mouseButton.button);
                if (!Input::global_state.count(code)) Input::global_state.insert(std::pair(code, Input::KeyState::Press));
            }
            else if (event.type == sf::Event::MouseButtonReleased)
            {
                const auto code = getSFMLMouse(event.mouseButton.button);
                if (Input::global_state.count(code)) Input::global_state.at(code) = Input::KeyState::Release;
            }
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

        top_scene->scripts.each([&](flecs::entity e, Script& script)
        {
            if (!e.is_alive() || e.has<Dead>()) return;

            // Execute the update function
            auto ent = Engine::Entity().asTable();
            ent.set("entity", e.raw_id());
            ent.set("good", true);
            ent.set("world", (uint64_t)world.c_ptr()); // Currently hacky way to store a pointer (must be considered an int64)

            auto _world = Engine::World().asTable();
            _world.set("world", (uint64_t)world.c_ptr());
            _world.set("good", true);

            #define CHECK_FUNCTION(name) \
                const auto ret = script.first->template runFunction<>(name, _world, ent);               \
                if (!ret && ret.error().code() != Lua::Runtime::ErrorCode::NotFunction)                 \
                    Log::Logger::instance("engine")->error("Lua Update(...) error ({}) in \"{}\": {}",  \
                        (int)ret.error().code(),                                                        \
                        script.first->filename(),                                                       \
                        ret.error().message())                                                          


            for (auto& script : script.runtime)
            {
                S2D_ASSERT(script.first, "Script runtime is null!");
                if (!script.second) 
                { 
                    CHECK_FUNCTION("Start");
                    script.second = true; 
                }
                CHECK_FUNCTION("Update");
            }

            #undef CHECK_FUNCTION

            // Check if it has a collider component and execute the collision function
        });

        if (world.count<Dead>())
        {
            std::vector<flecs::entity> dead_entities;
            dead_entities.reserve(top_scene->dead.count());
            top_scene->dead.each([&](flecs::entity e, Dead d)
                { dead_entities.push_back(e); }
            );
            for (auto& e : dead_entities) e.destruct();
        }

        window.clear();

        world.progress();

        render(top_scene);
        
        // Since colliders possibly change the velocity, we need to run collision check and *then* 
        // do the rigidbody transformation
        collide(top_scene);
        top_scene->rigidbodies.each([&](Transform& transform, Rigidbody& rigidbody)
        {
            rigidbody.velocity += (rigidbody.added_force - rigidbody.linear_drag * rigidbody.velocity) * (float)Time::dt;
            transform.position += sf::Vector3f(rigidbody.velocity.x, rigidbody.velocity.y, 0.f) * (float)Time::dt;
        });

        top_scene->draw(window);

        window.display();

        // Set dt
        auto now = std::chrono::high_resolution_clock::now();
        Time::dt = std::chrono::duration_cast<std::chrono::microseconds>(now - tick).count() / 1e6;
        if (Time::dt > 1.0) Time::dt = 1.0;
        tick = now;

        // Set mouse position
        Input::mouse_position = (sf::Vector2f)sf::Mouse::getPosition(window);
    }
}

Core::Core(const Application& app) :
    window(
        sf::VideoMode(app.size),
        app.name)
{   }

Core::~Core()
{
    while (_scenes.size()) { delete _scenes.top(); _scenes.pop(); }
}
}

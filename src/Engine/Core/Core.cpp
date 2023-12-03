#include <Simple2D/Engine/Core.hpp>

#include <Simple2D/Engine/LuaLib/ResLib.hpp>
#include <Simple2D/Engine/LuaLib/Input.hpp>
#include <Simple2D/Engine/LuaLib/Entity.hpp>
#include <Simple2D/Engine/LuaLib/World.hpp>
#include <Simple2D/Engine/LuaLib/Time.hpp>

#include <Simple2D/Engine/Application.hpp>

#include <Simple2D/Log/Log.hpp>

namespace S2D::Engine
{

Scene::Scene() :
    scripts(
        world.query_builder<Script>().build()
    ),
    colliders(
        world.query_builder<const Collider, Transform, Rigidbody>().build()
    ),
    dead(
        world.query_builder<Dead>().build()
    ),
    rigidbodies(
        world.query_builder<Transform, Rigidbody>().build()
    ),
    renderer(
        std::make_unique<Renderer>(this)
    )
{
    // Shaders should be in the renderer, and there should be different ones for each component type
    // Change the const shader pointer to a non-const and change the uniforms as needed there
    // load default shader
    //S2D_ASSERT(sf::Shader::isAvailable(), "Shaders are not supported on this machine");

    // For some wack reason the matrix multiplication *does not* translate

    //S2D_ASSERT(default_shader.loadFromMemory(vertex, fragment), "Error loading default shader");
}

Scene* Core::getTopScene()
{
    return _scenes.top();
}

void Core::run()
{
    using namespace Graphics;

    auto tick = std::chrono::high_resolution_clock::now();

    std::vector<double> frame_times(120);

    uint32_t frame = 0;
    while (window.isOpen() && _scenes.size())
    {
        Event event;

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
            if (event.type == Event::Type::Close)
                window.close();

            if (event.type == Event::Type::KeyPress)
            {
                const auto code = std::string(*event.keyPress.key);
                if (!Input::global_state.count(code)) Input::global_state.insert(std::pair(code, Input::KeyState::Press));
            }
            else if (event.type == Event::Type::KeyRelease)
            {
                const auto code = std::string(*event.keyPress.key);
                if (Input::global_state.count(code)) Input::global_state.at(code) = Input::KeyState::Release;
            }
            else if (event.type == Event::Type::MousePress)
            {
                const auto code = std::string(*event.mousePress.button);
                if (!Input::global_state.count(code)) Input::global_state.insert(std::pair(code, Input::KeyState::Press));
            }
            else if (event.type == Event::Type::MouseRelease)
            {
                const auto code = std::string(*event.mousePress.button);
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
        auto camera = world.filter<const Camera>().first();

        top_scene->scripts.each([&](flecs::entity e, Script& script)
        {
            if (!e.is_alive() || e.has<Dead>()) return;

            // Execute the update function
            auto ent = Engine::Entity().asTable();
            ent.set("entity", e.raw_id());
            ent.set("good", true);
            ent.set("world", (uint64_t)world.c_ptr()); // Currently hacky way to store a pointer (must be considered an int64)

            auto _world = Engine::World().asTable();
            _world.superimpose(Engine::ResLib().asTable());
            _world.set("world", (uint64_t)world.c_ptr());
            _world.set("scene", (uint64_t)top_scene);
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
        top_scene->update();

        render(top_scene);
        
        // Since colliders possibly change the velocity, we need to run collision check and *then* 
        // do the rigidbody transformation
        collide(top_scene);
        top_scene->rigidbodies.each([&](Transform& transform, Rigidbody& rigidbody)
        {
            rigidbody.velocity += (rigidbody.added_force - rigidbody.velocity * rigidbody.linear_drag) * (float)Time::dt;
            transform.position += Math::Vec3f(rigidbody.velocity.x, rigidbody.velocity.y, 0.f) * (float)Time::dt;
        });

        window.display();

        // Set dt
        auto now = std::chrono::high_resolution_clock::now();
        Time::dt = std::chrono::duration_cast<std::chrono::microseconds>(now - tick).count() / 1e6;
        if (Time::dt > 1.0) Time::dt = 1.0;
        tick = now;
        frame_times[frame % frame_times.size()] = Time::dt;

        // Set mouse position
        Math::Vec2f camera_pos;
        if (camera.is_alive() && camera.has<Transform>())
        {
            const auto* transform = camera.get<Transform>();
            camera_pos = { transform->position.x, transform->position.y };
        }

        // Won't need this hacky stuff once we start using shaders and pass camera transform info to them
        Input::mouse_position = Graphics::Mouse::getPosition() + camera_pos - (Math::Vec2f)window.getSize() / 2.f;

        frame++;

        if (frame % frame_times.size() == 0)
        {
            double avg = 0.f;
            for (const auto& time : frame_times) avg += time;
            avg /= (double)frame_times.size();
            Log::Logger::instance("engine")->trace("Last {} frames ran at {:0.1f} fps", frame_times.size(), 1.0 / avg);
        }
    }
}

Core::Core(const Application& app) :
    window(app.size, app.name)
{   
    Log::Logger::instance("engine")->info("Core and context started");
}

Core::~Core()
{
    while (_scenes.size()) { delete _scenes.top(); _scenes.pop(); }
}
}

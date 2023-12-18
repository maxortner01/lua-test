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
    dead(
        world.query_builder<Dead>().build()
    ),
    rigidbodies(
        world.query_builder<Transform, Rigidbody>().build()
    ),
    renderer(
        std::make_unique<Renderer>(this)
    )
{   }

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
    while (window.isOpen())
    {
        Scene* top_scene = nullptr;
        while (top_scene && _scenes.size())
        {
            auto* scene = getTopScene();
            if (!scene->running)
            {
                delete scene;
                _scenes.pop();
            }
            else
                top_scene = scene;
        }

#   ifndef USE_IMGUI
        if (!top_scene) break;
#   endif

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

            if (!top_scene) continue;

            if (event.type == Event::Type::KeyPress)
            {
                const auto code = std::string(*event.keyPress.key);
                if (!Input::global_state.count(code)) Input::global_state.insert(std::pair(code, Input::KeyState::Press));
                top_scene->runFunction<>("KeyPress");
            }
            else if (event.type == Event::Type::KeyRelease)
            {
                const auto code = std::string(*event.keyPress.key);
                if (Input::global_state.count(code)) Input::global_state.at(code) = Input::KeyState::Release;
                top_scene->runFunction<>("KeyRelease");
            }
            else if (event.type == Event::Type::MousePress)
            {
                const auto code = std::string(*event.mousePress.button);
                if (!Input::global_state.count(code)) Input::global_state.insert(std::pair(code, Input::KeyState::Press));
                top_scene->runFunction<>("MousePress");
            }
            else if (event.type == Event::Type::MouseRelease)
            {
                const auto code = std::string(*event.mousePress.button);
                if (Input::global_state.count(code)) Input::global_state.at(code) = Input::KeyState::Release;
                top_scene->runFunction<>("MouseRelease");
            }
        }

        // We execute all the scripts
        // THEN the other registered systems run with world.progress()
        // THEN the draw method is called

        if (top_scene)
        {

        auto& world = top_scene->world;
        auto camera = world.filter<const Camera>().first();

        top_scene->scripts.each([&](flecs::entity e, Script& script)
        {
            if (!e.is_alive() || e.has<Dead>()) return;

            void* t = world.c_ptr();
            // Execute the update function
            auto ent = Engine::Entity().asTable();
            ent.set("entity", (void*)e.raw_id());
            ent.set("good", true);
            ent.set("world", t); 

            auto _world = Engine::World().asTable();
            _world.superimpose(Engine::ResLib().asTable());
            _world.set("world", (void*)world.c_ptr());
            _world.set("scene", (void*)top_scene);
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

        // Since colliders possibly change the velocity, we need to run collision check and *then* 
        // do the rigidbody transformation
        top_scene->rigidbodies.each([&](Transform& transform, Rigidbody& rigidbody)
        {
            rigidbody.velocity += (rigidbody.added_force - rigidbody.velocity * rigidbody.linear_drag) * (float)Time::dt;
            transform.position += Math::Vec3f(rigidbody.velocity.x, rigidbody.velocity.y, 0.f) * (float)Time::dt;
        });
       
        world.progress();
        top_scene->update();

        }

        window.clear();

        if (top_scene) render(top_scene);
        else render_imgui(nullptr);

        window.display();

        // Set dt
        auto now = std::chrono::high_resolution_clock::now();
        Time::dt = std::chrono::duration_cast<std::chrono::microseconds>(now - tick).count() / 1e6;
        if (Time::dt > 1.0) Time::dt = 1.0;
        tick = now;
        frame_times[frame % frame_times.size()] = Time::dt;

        // Mouse position in window coordinates
        Input::mouse_position = [&]()
        {
            const auto window_pos = Graphics::Mouse::getPosition();
            return Math::Vec2f(
                (window_pos.x / (float)this->window.getSize().x - 0.5f) * 2.f,
                (window_pos.y / (float)this->window.getSize().y - 0.5f) * -2.f
            );
        }();

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
    S2D_ASSERT(fbo.create(app.size), "Failed to create final FBO");
    Log::Logger::instance("engine")->info("Core and context started");
}

Core::~Core()
{
    while (_scenes.size()) { delete _scenes.top(); _scenes.pop(); }
}
}

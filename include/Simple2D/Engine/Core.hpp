#pragma once

#include "../Util.hpp"

#include "Renderpass.hpp"
#include "Resources.hpp"
#include "Components.hpp"
#include "Renderer.hpp"

#include <stack>
#include <flecs.h>

namespace S2D::Engine
{
    struct Application;

    struct Scene : Util::NoCopy
    {
        // The container of entities
        flecs::world world;

        // Whether or not the core should destroy this scene
        bool running = true;

        // Where we load in the scene specific resources
        Resources resources;

        // Important cached queries for rendering
        flecs::query<Script> scripts; // for script system
        flecs::query<Transform, Rigidbody> rigidbodies; // for physics
        flecs::query<Dead> dead; // for cleanup

        std::unique_ptr<Renderer> renderer;
        std::unique_ptr<Renderpass> renderpass;

        Scene();
        virtual ~Scene() = default;

        virtual void start() { }
        virtual void update() { }
        virtual void constructPass(RenderpassBuilder& builder) { }
    };

    /**
     * @brief Represents the central executor of the game engine.
     */
    struct Core
    {
        Scene* getTopScene();

        template<typename T, typename... Args>
        T& emplaceScene(Args&&... args);

        double getDeltaTime() const;

        void run();

        Core(const Application& app);
        ~Core();

    private:
        void render(Scene* scene);
        void collide(Scene* scene);

        Graphics::DrawWindow window;

        std::stack<Scene*> _scenes;
    };

    template<typename T, typename... Args>
    T& Core::emplaceScene(Args&&... args)
    {
        static_assert(std::is_base_of_v<Scene, T>);
        T* scene = new T(std::forward<Args>(args)...);

        scene->start();

        RenderpassBuilder builder;

        // here is where we go through the cameras
        scene->world.template filter<const Camera>().each(
            [&](flecs::entity e, const Camera& camera)
            {
                builder.resource<Resource::Surface>({ std::string(e.name().c_str()), camera.size });
            });

        scene->constructPass(builder);
        scene->renderpass = builder.build();

        _scenes.push(scene);
        return *scene;
    }
}

#pragma once

#include "../Util.hpp"

#include "Resources.hpp"

#include <stack>
#include <flecs.h>
#include <SFML/Graphics.hpp>

namespace S2D::Engine
{
    struct Scene : Util::NoCopy
    {
        flecs::world world;
        bool running = true;
        Resources resources;

        Scene() = default;
        virtual ~Scene() = default;

        virtual void start() { };
        virtual void draw(sf::RenderTarget& target) {};
    };

    struct Display : Util::Singleton<Display>
    {
        sf::Vector2u size;
        std::string name;
    };

    struct Core : Util::Singleton<Core>
    {
        friend class Util::Singleton<Core>;

        Scene* getTopScene();

        template<typename T, typename... Args>
        T& emplaceScene(Args&&... args);

        // temp
        void loadFont(const std::string& name);

        double getDeltaTime() const;

        void run();

    private:
        Core();
        ~Core();

        void render(Scene* scene);

        double dt; // temp
        sf::Font font;
        sf::RenderWindow window;

        std::stack<Scene*> _scenes;
    };

    template<typename T, typename... Args>
    T& Core::emplaceScene(Args&&... args)
    {
        static_assert(std::is_base_of_v<Scene, T>);
        T* scene = new T(std::forward<Args>(args)...);

        scene->start();

        _scenes.push(scene);
        return *scene;
    }
}
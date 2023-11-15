#pragma once

#include <SFML/Graphics.hpp>

namespace S2D::Engine
{
    struct Core;

    struct Application
    {
        sf::Vector2u size;
        std::string name;

        Application(const sf::Vector2u& _size);
        virtual ~Application() = default;

        virtual void start(Core& core) = 0;
    };
}

extern S2D::Engine::Application* getApplication();

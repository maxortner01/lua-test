#pragma once

#include "../Util/Vector.hpp"

namespace S2D::Engine
{
    struct Core;

    struct Application
    {
        Math::Vec2u size;
        std::string name;

        Application(const Math::Vec2u& _size);
        virtual ~Application() = default;

        virtual void start(Core& core) = 0;
    };
}

extern S2D::Engine::Application* getApplication();

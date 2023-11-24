#pragma once

#include "Core.hpp"

namespace S2D::Engine
{

    struct LuaScene : Scene
    {
        LuaScene(const std::string& config_file);

        using Scene::constructPass;

        void start() override;

    private:
        void load_entities(const Lua::Table& entities);
        void load_resources(const Lua::Table& resources);

        Lua::Runtime runtime;
    };

}
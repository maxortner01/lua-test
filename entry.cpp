#include <Simple2D/Lua.hpp>
#include <Simple2D/Engine.hpp>

#include <sstream>
#include <iostream>
#include <chrono>
#include <numeric>

#include <flecs.h>

using namespace S2D;

#ifndef SOURCE_DIR
#define SOURCE_DIR "."
#endif

/*
struct MainScene : Engine::Scene
{
    void start() override
    {
        using namespace Engine;

        resources.loadResource<sf::Font>   ("arial", SOURCE_DIR "/fonts/arial.ttf"  );
        resources.loadResource<sf::Texture>("main",  SOURCE_DIR "/textures/test.png");
        resources.loadResource<sf::Texture>("tilemap",  SOURCE_DIR "/textures/Dungeon_Tileset.png");

        world.entity()
            .set(ComponentData<Name::Transform>{ .scale = 3.f, .position = { 100.f, 100.f, 0.f } })
            .set(ComponentData<Name::Tilemap>{ .tilesize = { 16.f, 16.f }, .spritesheet = { .texture_name = "tilemap" } })
            .set(loadScript(SOURCE_DIR "/scripts/tilemap.lua", world));

        world.entity()
            .set(loadScript(SOURCE_DIR "/scripts/fps.lua", world))
            .set(ComponentData<Name::Transform>{ .position = { 10.f, 20.f, 0.f }, .rotation = 0.f })
            .set(ComponentData<Name::Text>{ .string = "Hello", .font = "arial", .character_size = 16 });

        world.entity()
            .set(ComponentData<Name::Transform>{ .position = { 500.f, 100.f, -10.f }, .rotation = 0.f })
            .set(ComponentData<Name::Sprite>{ .size = sf::Vector2u(32, 32), .texture = "main" });
    }
};*/

struct LuaScene : Engine::Scene
{
    LuaScene(const std::string& config_file) :
        runtime(config_file)
    {   }

    void load_entities(const Lua::Table& entities)
    {
        uint32_t i = 1;
        while (entities.hasValue(std::to_string(i)))
        {
            const auto& entity_table = entities.get<Lua::Table>(std::to_string(i++));
            auto entity = (entity_table.hasValue("name")?world.entity(entity_table.get<Lua::String>("name").c_str()):world.entity());

            if (entity_table.hasValue("components"))
            {
                const auto& components = entity_table.get<Lua::Table>("components");

                uint32_t j = 1;
                while (components.hasValue(std::to_string(j)))
                {
                    const auto& component_table = components.get<Lua::Table>(std::to_string(j++));
                    const auto  id = (flecs::id_t)component_table.get<Lua::Number>("type");
                    const auto& value = component_table.get<Lua::Table>("value");
                    entity.add(id);
                    void* data = entity.get_mut(id);

                    Engine::setComponentFromTable(value, data, id, world);
                }
            }

            if (entity_table.hasValue("scripts"))
            {
                const auto& scripts = entity_table.get<Lua::Table>("scripts");

                uint32_t j = 1;
                while (scripts.hasValue(std::to_string(j)))
                {
                    // Check if its a string (just load it) or a table (which has a .filename and a .parameters)
                    // where the .paramters should be the global Parameters = {} object of that runtime
                    const auto& script_name = scripts.get<Lua::String>(std::to_string(j++));

                    if (!entity.has<Engine::Script>()) entity.set<Engine::Script>({});
                    auto* script = entity.get_mut<Engine::Script>();
                    Engine::loadScript(std::string(SOURCE_DIR) + script_name, world, *script);
                }
            }
        }
    }

    void load_resources(const Lua::Table& resources)
    {
        if (resources.hasValue("textures"))
        {
            const auto& textures = resources.get<Lua::Table>("textures");

            uint32_t i = 1;
            while (textures.hasValue(std::to_string(i)))
            {
                const auto& tex = textures.get<Lua::Table>(std::to_string(i++));
                const auto& name     = tex.get<Lua::String>("name");
                const auto& filename = tex.get<Lua::String>("location");
                this->resources.loadResource<sf::Texture>(name, std::string(SOURCE_DIR) + filename);
            }
        }

        if (resources.hasValue("fonts"))
        {
            const auto& fonts = resources.get<Lua::Table>("fonts");

            uint32_t i = 1;
            while (fonts.hasValue(std::to_string(i)))
            {
                const auto& font = fonts.get<Lua::Table>(std::to_string(i++));
                const auto& name     = font.get<Lua::String>("name");
                const auto& filename = font.get<Lua::String>("location");
                this->resources.loadResource<sf::Font>(name, std::string(SOURCE_DIR) + filename);
            }
        }
    }

    void start() override
    {
        Lua::Table globals;
        Engine::registerComponents(globals, world);
        runtime.setGlobal("Component", globals);

        auto res_r = runtime.runFunction<Lua::Table>("GetResources");
        if (res_r) load_resources(std::get<0>(res_r.value()));

        auto ent_res = runtime.runFunction<Lua::Table>("GetEntities");
        if (ent_res) load_entities(std::get<0>(ent_res.value()));
    }

    Lua::Runtime runtime;
};

struct App : Engine::Application
{
    App(const sf::Vector2u& size, const std::string& name) : Application(size)
    {   
        this->name = name;
    }

    void start(Engine::Core& core) override
    {
        core.emplaceScene<LuaScene>(SOURCE_DIR "/scripts/test.lua");
    }
};

Engine::Application* getApplication()
{
    /* Extract window size info from a config script */
    auto window = [&]()
    {
        Lua::Runtime runtime(SOURCE_DIR "/scripts/config.lua");
        auto res = runtime.getGlobal<Lua::Table>("Window");
        S2D_ASSERT(res, "Error loading the window information");
        return res.value();
    }();

    const auto size = [&]()
    {
        const auto& size = window.get<Lua::Table>("size");
        return sf::Vector2u({
            (unsigned int)size.get<Lua::Number>("width"),
            (unsigned int)size.get<Lua::Number>("height")
        });
    }();

    const auto& name = window.get<Lua::String>("title");

    return new App(size, name);
}
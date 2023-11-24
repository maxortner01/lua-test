#include <Simple2D/Engine/LuaScene.hpp>

namespace S2D::Engine
{

LuaScene::LuaScene(const std::string& config_file) :
    runtime(config_file)
{   }

void 
LuaScene::load_entities(const Lua::Table& entities)
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
                Engine::loadScript(script_name, world, *script);
            }
        }
    }
}

void 
LuaScene::load_resources(const Lua::Table& resources)
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
            this->resources.loadResource<sf::Texture>(name, filename);
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
            this->resources.loadResource<sf::Font>(name, filename);
        }
    }
}

void 
LuaScene::start() 
{
    /* Set the Component information */
    Lua::Table globals;
    Engine::registerComponents(globals, world);
    runtime.setGlobal("Component", globals);

    /* Directory */
    Lua::Table Directory;
    Directory.set<Lua::String>("Source", Script::SourceDir);
    runtime.setGlobal("Directory", Directory);

    auto res_r = runtime.runFunction<Lua::Table>("GetResources");
    if (res_r) load_resources(std::get<0>(res_r.value()));

    auto ent_res = runtime.runFunction<Lua::Table>("GetEntities");
    if (ent_res) load_entities(std::get<0>(ent_res.value()));
}

}
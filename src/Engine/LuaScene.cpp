#include <Simple2D/Engine/LuaScene.hpp>

#include <Simple2D/Log/Log.hpp>

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
    resources.try_get<Lua::Table>("textures", [&](const Lua::Table& textures)
    {
        textures.each<Lua::Table>([&](uint32_t i, const Lua::Table& texture)
        {
            const auto& name     = texture.get<Lua::String>("name");
            const auto& filename = texture.get<Lua::String>("location");
            this->resources.loadResource<sf::Texture>(name, filename);
        });
    });

    resources.try_get<Lua::Table>("fonts", [&](const Lua::Table& fonts)
    {
        fonts.each<Lua::Table>([&](uint32_t i, const Lua::Table& font)
        {
            const auto& name     = font.get<Lua::String>("name");
            const auto& filename = font.get<Lua::String>("location");
            this->resources.loadResource<sf::Font>(name, filename);
        });
    });

    resources.try_get<Lua::Table>("images", [&](const Lua::Table& images)
    {
        images.each<Lua::Table>([&](uint32_t i, const Lua::Table& image)
        {
            const auto& name     = image.get<Lua::String>("name");
            const auto& filename = image.get<Lua::String>("location");
            this->resources.loadResource<sf::Image>(name, filename);
        });
    });

    resources.try_get<Lua::Table>("shaders", [&](const Lua::Table& shaders)
    {
        shaders.each<Lua::Table>([&](uint32_t i, const Lua::Table& shader)
        {
            const auto& name = shader.get<Lua::String>("name");
            shader.try_get<Lua::String>("vertex", [&](const Lua::String& filename)
            { this->resources.loadResource<sf::Shader>(name, filename, sf::Shader::Vertex); });

            shader.try_get<Lua::String>("fragment", [&](const Lua::String& filename)
            { this->resources.loadResource<sf::Shader>(name, filename, sf::Shader::Fragment); });
        });
    });
}

void 
LuaScene::start() 
{
    auto& log = Log::Logger::instance("engine");

    prestart();

    /* Set the Component information */
    Lua::Table globals;
    Engine::registerComponents(globals, world);
    runtime.setGlobal("Component", globals);

    /* Directory */
    Lua::Table Directory;
    Directory.set<Lua::String>("Source", Script::SourceDir);
    runtime.setGlobal("Directory", Directory);

    Engine::globalEnum<Engine::Projection>(runtime, "ProjectionType");

    // Run the GetResources function and print any errors that occur
    auto res_r = runtime.runFunction<Lua::Table>("GetResources");
    if (res_r) load_resources(std::get<0>(res_r.value()));
    else if (!res_r && res_r.error().code() != Lua::Runtime::ErrorCode::NotFunction)
        log->error("In function GetResources ({}): {}", (int)res_r.error().code(), res_r.error().message());

    // Run the GetEntities function and print any errors that occur
    auto ent_res = runtime.runFunction<Lua::Table>("GetEntities");
    if (ent_res) load_entities(std::get<0>(ent_res.value()));
    else if (!ent_res && ent_res.error().code() != Lua::Runtime::ErrorCode::NotFunction)
        log->error("In function GetEntities ({}): {}", (int)ent_res.error().code(), ent_res.error().message());

    poststart();
}

}
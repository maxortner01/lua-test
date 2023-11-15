#include <Simple2D/Engine/Components.hpp>
#include <Simple2D/Engine/Time.hpp>
#include <Simple2D/Engine/Entity.hpp>
#include <Simple2D/Log/Library.hpp>
#include <Simple2D/Log/Log.hpp>

#include <flecs.h>

namespace S2D::Engine
{


Script loadScript(const std::string& filename, flecs::world& world)
{
    return Script {
        .runtime = std::make_unique<Lua::Runtime>([&]()
        {
            auto runtime = Lua::Runtime::create<
                Log::Library, Time
            >(filename);

            Lua::Table globals;
            registerComponents(globals, world);
            runtime.setGlobal("Component", globals);

            return runtime;
        }()),
        .initialized = false
    };
}

/* Position */
Lua::Table 
Component<Name::Transform>::getTable(
    const Data& data)
{
    Lua::Table table;

    Lua::Table position;
    position.set("x", data.position.x);
    position.set("y", data.position.y);
    position.set("z", data.position.z);
    table.set("position", position);
    table.set("rotation", data.rotation);

    return table;
}

void
Component<Name::Transform>::fromTable(
    const Lua::Table& table,
    void* _data)
{
    auto* data = reinterpret_cast<Data*>(_data);

    const auto& position = table.get<Lua::Table>("position");
    data->position.x = position.get<float>("x");
    data->position.y = position.get<float>("y");
    data->position.z = position.get<float>("z");
    data->rotation = table.get<Lua::Number>("rotation");
}

/* Rigidbody */
Lua::Table 
Component<Name::Rigidbody>::getTable(
    const ComponentData<Name::Rigidbody>& data)
{
    Lua::Table table;
    Lua::Table velocity;
    velocity.set("x", data.velocity.x);
    velocity.set("y", data.velocity.y);
    table.set("velocity", velocity);
    return table;
}

void
Component<Name::Rigidbody>::fromTable(
    const Lua::Table& table,
    void* _data)
{
    auto* data = reinterpret_cast<Data*>(_data);
    const auto& velocity = table.get<Lua::Table>("velocity");
    data->velocity.x = velocity.get<float>("x");
    data->velocity.y = velocity.get<float>("y");
}

Lua::Table 
Component<Name::Sprite>::getTable(
    const Data& data)
{
    Lua::Table table;

    Lua::Table size;
    size.set("width",  (Lua::Number)data.size.x);
    size.set("height", (Lua::Number)data.size.y);
    table.set("size", size);
    table.set("texture", data.texture);

    return table;
}

void 
Component<Name::Sprite>::fromTable(
    const Lua::Table& table, 
    void* _data)
{
    auto* data = reinterpret_cast<Data*>(_data);
    const auto& size = table.get<Lua::Table>("size");
    data->size.x = size.get<Lua::Number>("x");
    data->size.y = size.get<Lua::Number>("y");
    data->texture = table.get<Lua::String>("texture");
}

Lua::Table 
Component<Name::Text>::getTable(
    const Data& data)
{
    Lua::Table table;
    table.set("string", data.string);
    table.set("font",   data.font);
    table.set("characterSize", data.character_size);
    return table;
}

void 
Component<Name::Text>::fromTable(
    const Lua::Table& table, 
    void* _data)
{
    auto* data = reinterpret_cast<Data*>(_data);
    data->string = table.get<Lua::String>("string");
    data->font   = table.get<Lua::String>("font");
    data->character_size = table.get<Lua::Number>("characterSize");
}

int 
Component<Name::Tilemap>::setTile(
    Lua::State L)
{
    const auto [ component_table ] = Lua::Lib::Base::extractArgs<Lua::Table>(L);
    auto [ world, entity ] = Entity::extractWorldInfo(component_table);

    S2D_ASSERT(entity.is_alive(), "Entity is dead");

    const auto WORLD_ID = world.component<ComponentData<Name::Tilemap>>().raw_id();
    S2D_ASSERT(WORLD_ID == (decltype(WORLD_ID))component_table.get<Lua::Number>("type"), "Component is not Tilemap");
    auto* tilemap = entity.get_mut<ComponentData<Name::Tilemap>>();

    lua_pushboolean(L, false);

    return 1;
}

Lua::Table 
Component<Name::Tilemap>::getTable(
    const Data& data)
{
    Lua::Table table;
    table.set("setTile", (Lua::Function)setTile);
    return table;
}

void 
Component<Name::Tilemap>::fromTable(
    const Lua::Table& table, 
    void* _data)
{
    auto* data = reinterpret_cast<Data*>(_data);

}

void
registerComponents(Lua::Table& table, flecs::world& world)
{
    Util::CompileTime::static_for<(int)Name::Count>([&](auto n)
    {
        constexpr std::size_t i = n;
        constexpr auto component = static_cast<Name>(i);
        using Data = ComponentData<component>;
        table.set(std::string(*component), (Lua::Number)world.component<Data>().raw_id());
    });    
}

}
#include <Simple2D/Engine/Components.hpp>
#include <Simple2D/Log/Library.hpp>

#include <flecs.h>

namespace S2D::Engine
{


Script loadScript(const std::string& filename, flecs::world& world)
{
    return Script {
        .runtime = std::make_unique<Lua::Runtime>([&]()
        {
            auto runtime = Lua::Runtime::create<
                Log::Library
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
Component<Name::Position>::getTable(
    const ComponentData<Name::Position>& data)
{
    Lua::Table table;
    table.set("x", data.x);
    table.set("y", data.y);
    return table;
}

void
Component<Name::Position>::fromTable(
    const Lua::Table& table,
    void* _data)
{
    auto* data = reinterpret_cast<Data*>(_data);
    data->x = table.get<float>("x");
    data->y = table.get<float>("y");
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
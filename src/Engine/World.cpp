#include <Simple2D/Engine/World.hpp>
#include <Simple2D/Engine/Components.hpp>
#include <Simple2D/Engine/Entity.hpp>

#include <flecs.h>
#include <iostream>

namespace S2D::Engine
{

int World::createEntity(Lua::State L)
{
    using namespace Lua::CompileTime;
    using namespace Util::CompileTime;

    const std::size_t components = lua_gettop(L);
    std::vector<Lua::Table> tables;
    tables.reserve(components - 1);

    for (uint32_t i = 0; i < components - 1; i++)
    {
        assert(TypeMap<Lua::Table>::check(L));
        tables.emplace_back(L);
    }

    Lua::Table world_table(L);
    flecs::world world((flecs::world_t*)*world_table.get<uint64_t*>("id"));

    auto entity = world.entity();

    for (const auto& table : tables)
    {
        const std::size_t component_id = table.get<Lua::Number>("type");

        bool found = false;
        static_for<(int)Name::Count>([&](auto n)
        {
            if (found) return;

            constexpr std::size_t i = n;
            constexpr auto component = static_cast<Name>(i);
            using Data = ComponentData<component>;
            const std::size_t this_component_id = world.component<Data>().raw_id();
            if (this_component_id == component_id)
            {
                // Found the component
                found = true;

                Data data{0};
                if (table.hasValue("value"))
                {
                    const auto& value = table.get<Lua::Table>("value");
                    Component<component>::fromTable(value, (void*)&data);
                }
                entity.set(data);
            }
        });
    }

    auto entity_table = Entity().asTable();
    entity_table.set("good", true);
    entity_table.set("id", entity.raw_id());
    entity_table.set("world", (uint64_t)world.c_ptr()); // Currently hacky way to store a pointer (must be considered an int64)
    entity_table.toStack(L);

    return 1;
}

World::World() : Lua::Lib::Base("World",
    {
        { "createEntity", World::createEntity }
    })
{   }

}
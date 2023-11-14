#include <Simple2D/Engine/Entity.hpp>
#include <Simple2D/Engine/Components.hpp>

#include <flecs.h>

namespace S2D::Engine
{
    int Entity::getComponent(Lua::State L)
    {
        const auto [entity_table, component_id] = extractArgs<Lua::Table, Lua::Number>(L);

        // Extract the entity id and the world pointer from the entity table
        const auto e_id = *entity_table.get<uint64_t*>("id");
        const auto world_ptr = (flecs::world_t*)*entity_table.get<uint64_t*>("world");

        // Construct the world and the entity
        flecs::world world(world_ptr);
        flecs::entity entity(world_ptr, e_id);

        const std::size_t ID = component_id;

        using namespace Util::CompileTime;

        Lua::Table table;

        bool found = false;
        static_for<(int)Name::Count>([&](auto n)
        {
            if (found) return;

            constexpr std::size_t i = n;
            constexpr auto component = static_cast<Name>(i);
            auto id = world.component<ComponentData<component>>().raw_id();
            if (id == ID)
            {
                // Found
                const void* comp = entity.get(id);
                assert(comp);
                table.fromTable(Component<component>::getTable(*(const ComponentData<component>*)comp));
                table.set("good", true);
                table.set("type", (Lua::Number)ID);
                found = true;
            }
        });

        if (!found)
        {
            table.set("good", false);
            table.set("what", std::string("Entity doesn't have component"));
            table.toStack(L);
            return 1;
        }

        table.toStack(L);
        return 1;
    }

    int Entity::setComponent(Lua::State L)
    {
        using namespace Util::CompileTime;

        const auto [entity_table, component_table] = extractArgs<Lua::Table, Lua::Table>(L);

        assert(component_table.get<Lua::Boolean>("good"));

        // Extract the entity id and the world pointer from the entity table
        const auto id = *entity_table.get<uint64_t*>("id");
        const auto world_ptr = (flecs::world_t*)*entity_table.get<uint64_t*>("world");

        // Get the world
        flecs::world world(world_ptr);

        // Get the entity
        flecs::entity entity(world, id);

        // Make sure the component is in the entity
        const auto component_id = component_table.get<Lua::Number>("type");
        const auto& table = component_table;

        bool found = false;
        static_for<(int)Name::Count>([&](auto n)
        {
            if (found) return;

            constexpr std::size_t i = n;
            constexpr auto component = static_cast<Name>(i);
            auto id = world.component<ComponentData<component>>().raw_id();
            if (id == component_id)
            {
                // Found
                void* comp = entity.get_mut(id);
                Component<component>::fromTable(table, comp);
                found = true;
            }
        });

        if (!found) lua_pushboolean(L, false);
        else        lua_pushboolean(L, true);
        return 1;
    }

    int Entity::destroy(Lua::State L)
    {
        assert(lua_gettop(L) == 1);
        Lua::Table entity(L);

        const auto id = *entity.get<uint64_t*>("id");
        const auto world_ptr = (flecs::world_t*)*entity.get<uint64_t*>("world");

        flecs::entity e(world_ptr, id);
        if (e.is_alive() && !e.has<Dead>()) e.add<Dead>();

        return 0;
    }

    Entity::Entity() : Base("Entity",
        {
            { "getComponent", Entity::getComponent },
            { "setComponent", Entity::setComponent },
            { "destroy",      Entity::destroy      }
        })
    {   }
}

#include <Simple2D/Engine/Entity.hpp>
#include <Simple2D/Engine/Components.hpp>

#include <flecs.h>

namespace S2D::Engine
{
    int Entity::getComponent(Lua::State L)
    {
        const auto [entity_table, component_id] = extractArgs<Lua::Table, Lua::Number>(L);
        const auto e_id = *entity_table.get<uint64_t*>("id");

        auto& world = Util::Singleton<flecs::world>::get();

        flecs::entity entity(world.c_ptr(), e_id);

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
                table.fromTable(Component<component>::getTable(*(const ComponentData<component>*)comp));
                table.set("good", true);
                table.set("type", (Lua::Number)ID);
                //table.set("world", (uint64_t)world.c_ptr());
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

        auto& world = Util::Singleton<flecs::world>::get();

        // Get the entity
        flecs::entity entity(world.c_ptr(), *entity_table.get<uint64_t*>("id"));

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

    Entity::Entity() : Base("Entity",
        {
            { "getComponent", Entity::getComponent },
            { "setComponent", Entity::setComponent }
        })
    {   }
}
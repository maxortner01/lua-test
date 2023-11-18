#include <Simple2D/Engine/Entity.hpp>
#include <Simple2D/Engine/Components.hpp>
#include <Simple2D/Log/Library.hpp>
#include <Simple2D/Def.hpp>

#include <flecs.h>

namespace S2D::Engine
{

    std::pair<flecs::world, flecs::entity>
    Entity::extractWorldInfo(
        const Lua::Table& entity_or_component)
    {
        const auto e_id  = *entity_or_component.get<uint64_t*>("entity");
        const auto w_ptr = (flecs::world_t*)*entity_or_component.get<uint64_t*>("world");

        return {
            flecs::world(w_ptr),
            flecs::entity(w_ptr, e_id)
        };
    }

    int Entity::getComponent(Lua::State L)
    {
        const auto [entity_table, component_id] = extractArgs<Lua::Table, Lua::Number>(L);
        const auto world_info = extractWorldInfo(entity_table);
        const auto& world  = std::get<0>(world_info);
        const auto& entity = std::get<1>(world_info);

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
                S2D_ASSERT(comp, "Component is null");
                table.fromTable(Component<component>::getTable(*(const ComponentData<component>*)comp));
                table.set("good", true);
                table.set("type", (Lua::Number)ID);
                table.set("entity", entity.raw_id());
                table.set("world", (uint64_t)world.c_ptr());
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

        S2D_ASSERT(component_table.get<Lua::Boolean>("good"), "Component is not good");

        const auto world_info = extractWorldInfo(entity_table);
        const auto& world  = std::get<0>(world_info);
        const auto& entity = std::get<1>(world_info);

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
        const auto [ entity_table ] = extractArgs<Lua::Table>(L);
        auto [ world, entity ] = extractWorldInfo(entity_table);

        if (entity.is_alive() && !entity.has<Dead>()) entity.add<Dead>();

        return 0;
    }

    int Entity::addScript(Lua::State L)
    {
        const auto [entity_table, filename] = extractArgs<Lua::Table, Lua::String>(L);
        S2D_ASSERT(entity_table.get<Lua::Boolean>("good"), "Entity not good");
        auto [ world, entity ] = extractWorldInfo(entity_table);

        S2D_ASSERT(entity.is_alive(), "Entity is dead");
        if (!entity.has<Engine::Script>()) entity.set<Engine::Script>({});
        auto* script = entity.get_mut<Engine::Script>();
        loadScript(filename, world, *script);
        //entity.set(loadScript(filename, world));

        return 0;
    }

    Entity::Entity() : Base("Entity",
        {
            { "getComponent", Entity::getComponent },
            { "setComponent", Entity::setComponent },
            { "destroy",      Entity::destroy      },
            { "addScript",    Entity::addScript    }
        })
    {   }
}

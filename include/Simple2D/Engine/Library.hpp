#pragma once

#include <iostream>

#include "../Lua.hpp"
#include "Components.hpp"

#include <flecs.h>

namespace S2D::Engine
{
    struct Library : Lua::Lib::Base
    {
        static int getComponent(Lua::State L);
        static int setComponent(Lua::State L);

        Library();
    };

    int Library::getComponent(Lua::State L)
    {
        const auto [entity_table, component_id] = extractArgs<Lua::Table, Lua::Number>(L);
        const auto e_id = *entity_table.get<uint64_t*>("id");

        auto& world = Util::Singleton<flecs::world>::get();

        flecs::entity entity(world.c_ptr(), e_id);

        const std::size_t ID = component_id;

        using namespace Util::CompileTime;

        Lua::Table table;

        bool found = false;
        static_for<(int)COMPONENT_ENUM_NAME::Count>([&](auto n)
        {
            if (found) return;

            constexpr std::size_t i = n;
            constexpr auto component = static_cast<COMPONENT_ENUM_NAME>(i);
            auto id = world.component<typename COMPONENT_STRUCT_NAME<component>::Data>().raw_id();
            if (id == ID)
            {
                // Found
                const void* comp = entity.get(id);
                table.fromTable(COMPONENT_STRUCT_NAME<component>::getTable(*(const typename COMPONENT_STRUCT_NAME<component>::Data*)comp));
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

    int Library::setComponent(Lua::State L)
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
        static_for<(int)COMPONENT_ENUM_NAME::Count>([&](auto n)
        {
            if (found) return;

            constexpr std::size_t i = n;
            constexpr auto component = static_cast<COMPONENT_ENUM_NAME>(i);
            auto id = world.component<typename COMPONENT_STRUCT_NAME<component>::Data>().raw_id();
            if (id == component_id)
            {
                // Found
                void* comp = entity.get_mut(id);
                COMPONENT_STRUCT_NAME<component>::fromTable(table, comp);
                found = true;
            }
        });

        if (!found) lua_pushboolean(L, false);
        else        lua_pushboolean(L, true);
        return 1;
    }

    Library::Library() : Base("Entity",
        {
            { "getComponent", Library::getComponent },
            { "setComponent", Library::setComponent }
        })
    {   }
}
#pragma once

#include "../Lua.hpp"

#include <flecs.h>

#ifndef COMPONENT_ENUM_NAME
#ERROR "Must define a component enum class that details each component type avaliable"
#endif

#ifndef COMPONENT_STRUCT_NAME
#ERROR "Must define a component struct with the name enum as its template"
#endif

namespace S2D::Engine
{
#ifdef DOCS

/**
 * @brief This is documentation for the scripting API for Lua.
 */
namespace LuaAPI
{
    /**
     * @brief Represents the entity that a script is attached to
     * 
     */
    struct Entity
    {
        /**
         * @brief Get a component of a specific type from the entity
         * 
         * @param type The type of component
         * @return Component The component associated with that type
         */
        Component getComponent(ComponentType type);
        void setComponent(Component)
    };
}

#endif

    template<COMPONENT_ENUM_NAME T>
    using ComponentData = typename COMPONENT_STRUCT_NAME<T>::Data;

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
            auto id = world.component<ComponentData<component>>().raw_id();
            if (id == ID)
            {
                // Found
                const void* comp = entity.get(id);
                table.fromTable(COMPONENT_STRUCT_NAME<component>::getTable(*(const ComponentData<component>*)comp));
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
            auto id = world.component<ComponentData<component>>().raw_id();
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
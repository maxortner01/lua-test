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
        const auto [entity_table, component] = extractArgs<Lua::Table, std::string>(L);
        const auto e_id = *entity_table.get<uint64_t*>("id");

        auto& world = Util::Singleton<flecs::world>::get();

        flecs::entity entity(world.c_ptr(), e_id);

        bool found = false;
        uint64_t c_id;
        for (uint32_t i = 0; i < entity.type().count(); i++)
        {
            std::string name = entity.type().get(i).str().c_str();
            if (name == NAME_PRE + component + NAME_POST)
            {
                found = true;
                c_id = entity.type().get(i).raw_id();
                break;
            }
        }

        if (!found)
        {
            Lua::Table ret;
            ret.set("good", false);
            ret.set("what", std::string("Entity doesn't have component"));
            ret.toStack(L);
            return 1;
        }

        const void* comp = entity.get(c_id);
        auto ret = Comp::componentToTable(component, comp);
        ret.set("good", true);

        ret.toStack(L);

        return 1;
    }

    int Library::setComponent(Lua::State L)
    {
        const auto [entity_table, component_table] = extractArgs<Lua::Table, Lua::Table>(L);

        assert(component_table.get<Lua::Boolean>("good"));

        auto& world = Util::Singleton<flecs::world>::get();
        // Get the entity
        flecs::entity entity(world.c_ptr(), *entity_table.get<uint64_t*>("id"));

        // Make sure the component is in the entity
        const auto component = component_table.get<Lua::String>("name");

        bool found = false;
        uint64_t c_id;
        for (uint32_t i = 0; i < entity.type().count(); i++)
        {
            std::string name = entity.type().get(i).str().c_str();
            if (name == NAME_PRE + component + NAME_POST)
            {
                found = true;
                c_id = entity.type().get(i).raw_id();
                break;
            }
        }

        if (!found) lua_pushboolean(L, false);
        else
        {
            void* data = entity.get_mut(c_id);
            if (!Comp::tableToComponent(component_table, data))
                lua_pushboolean(L, false);
            else
                lua_pushboolean(L, true);
        }
        
        return 1;
    }

    Library::Library() : Base("Entity",
        {
            { "getComponent", Library::getComponent },
            { "setComponent", Library::setComponent }
        })
    {   }
}
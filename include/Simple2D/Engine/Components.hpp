#pragma once

#include "../Lua.hpp"

#ifndef COMPONENT_ENUM_NAME
#ERROR "Must define a component enum class that details each component type avaliable"
#endif

#ifndef COMPONENT_STRUCT_NAME
#ERROR "Must define a component struct with the name enum as its template"
#endif

namespace S2D::Comp
{
    // Requires that T has member T::Count and is castable to int and has const char* operator* defined
    // And that Component<(T)i>::fromTable is defined
    static bool
    tableToComponent(const Lua::Table& table, void* data)
    {
        using namespace Util::CompileTime;

        bool found = false;

        static_for<(int)COMPONENT_ENUM_NAME::Count>([&](auto n)
        {
            if (found) return;

            constexpr std::size_t i = n;
            constexpr auto component = static_cast<COMPONENT_ENUM_NAME>(i);
            const auto comp_name = std::string(*component);
            const auto& name = table.get<Lua::String>("name");
            
            if (name == comp_name)
            {
                found = true;
                COMPONENT_STRUCT_NAME<component>::fromTable(table, data);
            }
        });

        return found;
    }

    // Requires that T has member T::Count and is castable to int and has const char* operator* defined
    // And that Component<(T)i>::getTable is defined
    static Lua::Table
    componentToTable(const std::string& name, const void* data)
    {
        using namespace Util::CompileTime;

        Lua::Table table;
        bool found = false;

        static_for<(int)COMPONENT_ENUM_NAME::Count>([&](auto n)
        {
            if (found) return;

            constexpr std::size_t i = n;
            constexpr auto component = static_cast<COMPONENT_ENUM_NAME>(i);
            const auto& data_r = *reinterpret_cast<const typename COMPONENT_STRUCT_NAME<component>::Data*>(data);
            const auto comp_name = std::string(*component);

            if (name == comp_name)
            {
                const auto map = COMPONENT_STRUCT_NAME<component>::getTable(data_r);
                table.fromTable(map);
                table.set("name", comp_name);
                found = true;
            }
        });

        if (!found)
        {
            table.set("good", false);
            table.set("what", std::string("Component Type Doesn't Exist"));
        }
        else
            table.set("good", true);

        return table;
    }
}
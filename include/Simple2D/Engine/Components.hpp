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

        const auto name_id = table.get<Lua::Number>("type");

        if (name_id < 0 || name_id > (int)COMPONENT_ENUM_NAME::Count)
            return false;

        static_for<(int)COMPONENT_ENUM_NAME::Count>([&](auto n)
        {
            constexpr std::size_t i = n;
            
            if ((int)name_id != i) return;

            constexpr auto component = static_cast<COMPONENT_ENUM_NAME>(i);
            COMPONENT_STRUCT_NAME<component>::fromTable(table, data);
        });

        return true;
    }

    // Requires that T has member T::Count and is castable to int and has const char* operator* defined
    // And that Component<(T)i>::getTable is defined
    static Lua::Table
    componentToTable(Lua::Number name_id, const void* data)
    {
        using namespace Util::CompileTime;

        Lua::Table table;
        if (name_id < 0 || name_id > (int)COMPONENT_ENUM_NAME::Count)
        {
            table.set("good", false);
            table.set("what", std::string("Component type doesn't exist"));
            return table;
        }

        static_for<(int)COMPONENT_ENUM_NAME::Count>([&](auto n)
        {
            constexpr std::size_t i = n;

            if (i != (int)name_id) return;

            constexpr auto component = static_cast<COMPONENT_ENUM_NAME>(i);
            const auto& data_r = *reinterpret_cast<const typename COMPONENT_STRUCT_NAME<component>::Data*>(data);

            const auto map = COMPONENT_STRUCT_NAME<component>::getTable(data_r);
            table.fromTable(map);
            table.set("type", name_id);
        });

        return table;
    }
}
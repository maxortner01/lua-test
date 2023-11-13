#pragma once

#include "../Lua.hpp"

namespace S2D::Engine
{
    struct Script
    {
        std::unique_ptr<Lua::Runtime> runtime;
    };

    enum class Name
    {
        Position,
        Rigidbody,
        Count
    };

    static const char* operator*(Name name)
    {
        switch (name)
        {
        case Name::Position:  return "Position";
        case Name::Rigidbody: return "Rigidbody";
        default: return "";
        }
    }

    template<Name T>
    struct Component;

    template<>
    struct Component<Name::Position>
    {
        static constexpr Name Type = Name::Position;
        struct Data
        {
            float x, y;
        };

        static Lua::Table getTable(const Data& data);
        static void fromTable(const Lua::Table& table, void* _data);
    };

    template<>
    struct Component<Name::Rigidbody>
    {
        static constexpr Name Type = Name::Rigidbody;
        struct Data
        {
            struct {
                float x, y;
            } velocity;
        };

        static Lua::Table getTable(const Data& data);
        static void fromTable(const Lua::Table& table, void* _data);
    };

    template<Name T>
    using ComponentData = typename Component<T>::Data;

    void
    registerComponents(Lua::Table& table);
}
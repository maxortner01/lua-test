#pragma once

#include <Simple2D/Lua.hpp>
#include <memory>

using namespace S2D;

enum class Name
{
    Position,
    RigidBody,
    Count
};

const char* operator*(Name c)
{
    switch(c)
    {
    case Name::Position:  return "Position";
    case Name::RigidBody: return "RigidBody";
    default: return "";
    }
}


struct Script
{
    std::unique_ptr<Lua::Runtime> runtime;
};

template<Name T>
struct Component;

#define COMPONENT_ENUM_NAME Name
#define COMPONENT_STRUCT_NAME Component

template<>
struct Component<Name::Position>
{
    static constexpr Name Type = Name::Position;

    struct Data
    {
        float x, y;
    };

    static Lua::Table getTable(const Data& data)
    {
        Lua::Table table;
        table.set("x", data.x);
        table.set("y", data.y);
        return table;
    }

    static void fromTable(const Lua::Table& table, void* _data)
    {
        auto* data = reinterpret_cast<Data*>(_data);
        data->x = table.get<float>("x");
        data->y = table.get<float>("y");
    }
};

template<>
struct Component<Name::RigidBody>
{
    static constexpr Name Type = Name::RigidBody;

    struct Data
    {
        struct {
            float x, y;
        } velocity;
    };

    static Lua::Table getTable(const Data& data)
    {
        Lua::Table table;
        Lua::Table velocity;
        velocity.set("x", data.velocity.x);
        velocity.set("y", data.velocity.y);
        table.set("velocity", velocity);
        return table;
    }

    static void fromTable(const Lua::Table& table, void* _data)
    {
        auto* data = reinterpret_cast<Data*>(_data);
        const auto& velocity = table.get<Lua::Table>("velocity");
        data->velocity.x = velocity.get<float>("x");
        data->velocity.y = velocity.get<float>("y");
    }
};
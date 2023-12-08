#include <Simple2D/Engine/LuaLib/MeshLib.hpp>
#include <Simple2D/Engine/LuaLib/Entity.hpp>

#include <Simple2D/Engine/Mesh.hpp>
#include <Simple2D/Engine/Components.hpp>

#include <Simple2D/Log/Log.hpp>

#include "../../Lua/Lua.cpp"

#define LUA_EXCEPTION(expr, msg) if (!(expr)) { logger->error(msg); return 0; }

namespace S2D::Engine
{

int
MeshLib::setPrimitiveType(Lua::State L)
{
    auto& logger = Log::Logger::instance("engine");
    const auto [ mesh_table, primitive_type ] = extractArgs<Lua::Table, Lua::Number>(L);
    const auto [ world, entity ] = Entity::extractWorldInfo(mesh_table);
    LUA_EXCEPTION(entity.has<CustomMesh>(), "Entity missing custom mesh component");
    LUA_EXCEPTION(primitive_type <= (int)Primitive::Lines && primitive_type >= 0, "Incorrect primitive type given");

    auto* mesh = entity.get_mut<CustomMesh>();
    if (!mesh->mesh) mesh->mesh = std::make_unique<RawMesh>();

    mesh->mesh->primitive = (Primitive)primitive_type;

    return 0;
}

int 
MeshLib::getVertexCount(Lua::State L)
{
    auto& logger = Log::Logger::instance("engine");
    const auto [ mesh_table ] = extractArgs<Lua::Table>(L);
    const auto [ world, entity ] = Entity::extractWorldInfo(mesh_table);
    LUA_EXCEPTION(entity.has<CustomMesh>(), "Entity missing custom mesh component");
    
    auto* mesh = entity.get<CustomMesh>();
    lua_pushnumber(STATE, mesh->mesh ? mesh->mesh->vertices.vertexCount() : 0);
    return 1;
}

int 
MeshLib::pushVertex(Lua::State L)
{
    auto& logger = Log::Logger::instance("engine");
    const auto [ mesh_table, vertex ] = extractArgs<Lua::Table, Lua::Table>(L);
    const auto [ world, entity ] = Entity::extractWorldInfo(mesh_table);
    LUA_EXCEPTION(entity.has<CustomMesh>(), "Entity missing custom mesh component");

    auto* mesh = entity.get_mut<CustomMesh>();
    if (!mesh->mesh) mesh->mesh = std::make_unique<RawMesh>();

    Graphics::Vertex v;

    if (vertex.hasValue("position"))
    {
        const auto& position = vertex.get<Lua::Table>("position");
        v.position = {
            position.hasValue("x") ? position.get<Lua::Number>("x") : 0.f,
            position.hasValue("y") ? position.get<Lua::Number>("y") : 0.f,
            position.hasValue("z") ? position.get<Lua::Number>("z") : 0.f
        };
    }

    if (vertex.hasValue("color"))
    {
        const auto& color = vertex.get<Lua::Table>("color");
        v.color = Graphics::Color(
            (uint8_t)(int)(color.hasValue("r") ? color.get<Lua::Number>("r") : 0),
            (uint8_t)(int)(color.hasValue("g") ? color.get<Lua::Number>("g") : 0),
            (uint8_t)(int)(color.hasValue("b") ? color.get<Lua::Number>("b") : 0),
            (uint8_t)(int)(color.hasValue("a") ? color.get<Lua::Number>("a") : 0)
        );
    }

    if (vertex.hasValue("texpos"))
    {
        const auto& tex = vertex.get<Lua::Table>("texpos");
        v.texCoords = {
            tex.hasValue("x") ? tex.get<Lua::Number>("x") : 0.f,
            tex.hasValue("y") ? tex.get<Lua::Number>("y") : 0.f
        };
    }

    // need to add CPU side of vertexarray
    //mesh->mesh->vertices.append(v);

    return 0;
}

int 
MeshLib::getVertex(Lua::State L)
{
    return 0;
    /*
    auto& logger = Log::Logger::instance("engine");
    const auto [ mesh_table, index ] = extractArgs<Lua::Table, Lua::Number>(L);
    LUA_EXCEPTION(index >= 0, "Index can't be negative");

    const auto [ world, entity ] = Entity::extractWorldInfo(mesh_table);
    LUA_EXCEPTION(entity.has<CustomMesh>(), "Entity missing custom mesh component");

    const auto* mesh = entity.get<CustomMesh>();
    LUA_EXCEPTION(mesh->mesh, "Mesh not generated");

    const auto& vertex = mesh->mesh->vertices[(uint32_t)index];
    Lua::Table table;

    Lua::Table position;
    position.set("x", vertex.position.x);
    position.set("y", vertex.position.y);
    table.set("position", position);

    Lua::Table color;
    position.set("r", (Lua::Number)(int)vertex.color.r);
    position.set("g", (Lua::Number)(int)vertex.color.g);
    position.set("b", (Lua::Number)(int)vertex.color.b);
    position.set("a", (Lua::Number)(int)vertex.color.a);
    table.set("color", color);

    Lua::Table texpos;
    position.set("x", vertex.texCoords.x);
    position.set("y", vertex.texCoords.y);
    table.set("texpos", texpos);

    table.toStack(L);

    return 1;*/
}

MeshLib::MeshLib() : Base("Mesh",
    {
        { "setPrimitiveType", MeshLib::setPrimitiveType },
        { "getVertexCount",   MeshLib::getVertexCount   },
        { "pushVertex",       MeshLib::pushVertex       },
        { "getVertex",        MeshLib::getVertex        }
    })
{   }

}

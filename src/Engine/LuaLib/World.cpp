#include <Simple2D/Engine/LuaLib/World.hpp>
#include <Simple2D/Engine/LuaLib/Entity.hpp>

#include <Simple2D/Engine/Components.hpp>
#include <Simple2D/Def.hpp>

#include <flecs.h>

namespace S2D::Engine
{

static int createEntityFromComponents(Lua::State L)
{
    using namespace Lua::CompileTime;
    using namespace Util::CompileTime;
    
    const std::size_t components = lua_gettop(L);
    
    std::vector<Lua::Table> tables;
    tables.reserve(components - 1);

    for (uint32_t i = 0; i < components - 1; i++)
    {
        S2D_ASSERT(TypeMap<Lua::Table>::check(L), "Lua type mismatch");
        tables.emplace_back(L);
    }

    Lua::Table world_table(L);
    flecs::world world((flecs::world_t*)world_table.get<void*>("world"));

    auto entity = world.entity();

    for (const auto& table : tables)
    {
        const std::size_t component_id = table.get<Lua::Number>("type");

        bool found = false;
        static_for<(int)Name::Count>([&](auto n)
        {
            if (found) return;

            constexpr std::size_t i = n;
            constexpr auto component = static_cast<Name>(i);
            using Data = ComponentData<component>;
            const std::size_t this_component_id = world.component<Data>().raw_id();
            if (this_component_id == component_id)
            {
                // Found the component
                found = true;

                Data data;
                if (table.hasValue("value"))
                {
                    const auto& value = table.get<Lua::Table>("value");
                    Component<component>::fromTable(value, (void*)&data);
                }
                entity.set(data);
            }
        });
    }

    auto entity_table = Entity().asTable();
    entity_table.set("good", true);
    entity_table.set("entity", (void*)entity.raw_id());
    entity_table.set("world", (void*)world.c_ptr()); 
    entity_table.toStack(L);

    return 1;
}

static int createEntityInitComponents(Lua::State L)
{
    using namespace Lua::CompileTime;
    using namespace Util::CompileTime;

    const std::size_t components = lua_gettop(L);
    std::vector<Lua::Number> numbers(components);
    for (uint32_t i = 0; i < components - 1; i++)
    {
        S2D_ASSERT(lua_type(L, -1) == LUA_TNUMBER, "Type mismatch");
        numbers[i] = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    
    S2D_ASSERT(lua_gettop(L) == 1, "Argument size mismatch");
    Lua::Table world_table(L);
    flecs::world world((flecs::world_t*)world_table.get<void*>("world"));
    
    auto entity = world.entity();
    
    for (const auto& id : numbers)
    {
        const std::size_t component_id = id;

        bool found = false;
        static_for<(int)Name::Count>([&](auto n)
        {
            if (found) return;

            constexpr std::size_t i = n;
            constexpr auto component = static_cast<Name>(i);
            using Data = ComponentData<component>;
            const std::size_t this_component_id = world.component<Data>().raw_id();
            if (this_component_id == component_id)
            {
                // Found the component
                found = true;
                entity.set<Data>({});
            }
        });
    }
    
    auto entity_table = Entity().asTable();
    entity_table.set("good", true);
    entity_table.set("entity", (void*)entity.raw_id());
    entity_table.set("world", (void*)world.c_ptr()); 
    entity_table.toStack(L);
    
    return 1;
}

int World::createEntity(Lua::State L)
{
    const std::size_t components = lua_gettop(L);
    S2D_ASSERT(lua_type(L, (int)(-1 * components)) == LUA_TTABLE, "Missing world");
    
    bool is_tables = true;
    bool is_init   = true;
    for (int32_t i = -1; i > -1 * components; i--)
    {
        if (lua_type(L, i) != LUA_TTABLE) is_tables = false;
        if (lua_type(L, i) != LUA_TNUMBER) is_init = false;
    }
    
    S2D_ASSERT(is_tables || is_init, "Arguments invalid");
    if (is_tables) return createEntityFromComponents(L);
    if (is_init)   return createEntityInitComponents(L);
    S2D_ASSERT(false, "Something went wrong");
    return 0;
}

int World::getEntity(Lua::State L)
{
    const auto [ world_table, name ] = extractArgs<Lua::Table, Lua::String>(L);

    S2D_ASSERT(world_table.hasValue("world"), "World table is messed up");
    flecs::world world((flecs::world_t*)*world_table.get<void**>("world"));

    auto entity = world.lookup(name.c_str());
    S2D_ASSERT(entity.is_alive(), "Entity is dead :(");

    auto ent = Engine::Entity().asTable();
    ent.set("entity", (void*)entity.raw_id());
    ent.set("good", true);
    ent.set("world", (void*)world.c_ptr()); // Currently hacky way to store a pointer (must be considered an int64)
    S2D_ASSERT(!lua_gettop(L), "Unknown args");

    ent.toStack(L);
    return 1;
}

World::World() : Lua::Lib::Base("World",
    {
        { "createEntity", World::createEntity },
        { "getEntity",    World::getEntity    }
    })
{   }

}

#include <Simple2D/Lua/Table.hpp>
#include <Simple2D/Lua/TypeMap.hpp>

#include <cassert>

namespace S2D::Lua
{

/* Table::Data */
template<typename T>
Table::Data 
Table::Data::fromValue(const T& value)
{
    return Data {
        .data = emplace(value),
        .type = CompileTime::TypeMap<T>::LuaType
    };    
}
template Table::Data Table::Data::fromValue(const Lua::Number&);
template Table::Data Table::Data::fromValue(const Lua::String&);
template Table::Data Table::Data::fromValue(const Lua::Boolean&);
template Table::Data Table::Data::fromValue(const Lua::Function&);
template Table::Data Table::Data::fromValue(const Lua::Table&);
template Table::Data Table::Data::fromValue(void* const&);

template<typename T>
std::shared_ptr<void>
Table::Data::emplace(const T& value)
{
    std::shared_ptr<void> ptr = std::shared_ptr<void>(
        std::malloc(sizeof(T)),
        [](void* ptr) { std::free(ptr); }
    );
    *static_cast<T*>(ptr.get()) = value;
    return ptr;
}
template std::shared_ptr<void> Table::Data::emplace(const Lua::Number&);
template std::shared_ptr<void> Table::Data::emplace(const Lua::String&);
template std::shared_ptr<void> Table::Data::emplace(const Lua::Boolean&);
template std::shared_ptr<void> Table::Data::emplace(const Lua::Function&);
template std::shared_ptr<void> Table::Data::emplace(const Lua::Table&);

/* Table */

Table::Table(const Table::Map& map) :
    dictionary(map)
{   }

Table::Table(State L)
{
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        assert(lua_type(L, -2) == LUA_TSTRING);
        
        std::string key = lua_tostring(L, -2);
        std::shared_ptr<void> value;

        const auto count = lua_gettop(L);
        const auto type = lua_type(L, -1);
        switch(type)
        {
        case LUA_TNUMBER:   value = Data::emplace(static_cast<Lua::Number>(lua_tonumber(L, -1))); break;
        case LUA_TSTRING:   value = Data::emplace(Lua::String(lua_tostring(L, -1)));              break;
        case LUA_TBOOLEAN:  value = Data::emplace(lua_toboolean(L, -1));                          break;
        case LUA_TUSERDATA: value = Data::emplace(lua_touserdata(L, -1));                         break;
        case LUA_TTABLE:    value = Data::emplace(Table(L)); break;
        }
        
        dictionary.insert(std::pair(key, Data{ .data = value, .type = type }));
        if (count == lua_gettop(L)) lua_pop(L, 1);
    }
    lua_pop(L, 1);
}

void
Table::fromTable(const Table& table)
{
    dictionary = table.dictionary;
}

void 
Table::superimpose(const Table& table)
{
    superimpose(table.dictionary);
}

void 
Table::superimpose(const Map& map)
{
    for (const auto& p : map)
        dictionary.insert(p);
}

template<typename T>
T& Table::get(const std::string& name)
{
    assert(dictionary.count(name));
    return *static_cast<T*>(dictionary.at(name).data.get());
}
template Lua::Number&   Table::get(const std::string&);
template Lua::String&   Table::get(const std::string&);
template Lua::Boolean&  Table::get(const std::string&);
template Lua::Function& Table::get(const std::string&);
template Lua::Table&    Table::get(const std::string&);
template uint64_t*&     Table::get(const std::string&);

template<typename T>
const T& Table::get(const std::string& name) const
{
    assert(dictionary.count(name));
    return *static_cast<T*>(dictionary.at(name).data.get());
}
template const Lua::Number&   Table::get(const std::string&) const;
template const Lua::String&   Table::get(const std::string&) const;
template const Lua::Boolean&  Table::get(const std::string&) const;
template const Lua::Function& Table::get(const std::string&) const;
template const Lua::Table&    Table::get(const std::string&) const;
template uint64_t* const&     Table::get(const std::string&) const;

template<typename T>
void Table::set(const std::string& name, const T& value)
{
    dictionary.insert(std::pair(name, Table::Data::fromValue(value)));
}
template void Table::set(const std::string&, const Lua::Number&);
template void Table::set(const std::string&, const Lua::String&);
template void Table::set(const std::string&, const Lua::Boolean&);
template void Table::set(const std::string&, const Lua::Function&);
template void Table::set(const std::string&, const Lua::Table&);
template void Table::set(const std::string&, const uint64_t&);

const Table::Map&
Table::getMap() const
{ return dictionary; }

void
Table::toStack(State L) const
{
    using namespace CompileTime;

    lua_newtable(L);

    for (const auto& p : dictionary)
    {
        lua_pushstring(L, p.first.c_str());

        switch (p.second.type)
        {
        case LUA_TNUMBER:  TypeMap<Lua::Number> ::push(L, get<Lua::Number> (p.first)); break;
        case LUA_TSTRING:  TypeMap<Lua::String> ::push(L, get<Lua::String> (p.first)); break;
        case LUA_TBOOLEAN: TypeMap<Lua::Boolean>::push(L, get<Lua::Boolean>(p.first)); break;
        case LUA_TTABLE:
        {
            const auto& table = get<Lua::Table>(p.first);
            table.toStack(L);
            break;
        }
        case LUA_TFUNCTION: TypeMap<Lua::Function>::push(L, get<Lua::Function>(p.first)); break;
        default: TypeMap<uint64_t>::push(L, get<uint64_t>(p.first)); break; // need to make this general
        }

        lua_settable(L, -3);
    }
}

} // S2D::Lua
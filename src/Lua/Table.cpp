#include <Simple2D/Lua/Table.hpp>
#include <Simple2D/Lua/TypeMap.hpp>
#include <Simple2D/Def.hpp>

#include <iostream>

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
        new T(),
        [](void* ptr) { delete reinterpret_cast<T*>(ptr); }
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
        const auto key = [&]()
        {
            switch (lua_type(L, -2))
            {
            case LUA_TSTRING: return std::string(lua_tostring(L, -2));
            case LUA_TNUMBER: return std::to_string((int)lua_tonumber(L, -2));
            default: S2D_ASSERT(false, "Lua type mismatch");
            }
        }();
        
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

template<typename T>
void 
Table::each(std::function<void(uint32_t, T&)> lambda)
{
    uint32_t i = 1;
    while (hasValue(std::to_string(i)))
    {
        auto& val = get<T>(std::to_string(i));
        lambda(i++, val);
    }
}
template void Table::each(std::function<void(uint32_t, Lua::Number&)>);
template void Table::each(std::function<void(uint32_t, Lua::String&)>);
template void Table::each(std::function<void(uint32_t, Lua::Boolean&)>);
template void Table::each(std::function<void(uint32_t, Lua::Function&)>);
template void Table::each(std::function<void(uint32_t, Lua::Table&)>);
template void Table::each(std::function<void(uint32_t, uint64_t*&)>);

template<typename T>
void 
Table::each(std::function<void(uint32_t, const T&)> lambda) const
{
    uint32_t i = 1;
    while (hasValue(std::to_string(i)))
    {
        const auto& val = get<T>(std::to_string(i));
        lambda(i++, val);
    }
}
template void Table::each(std::function<void(uint32_t, const Lua::Number&)>) const;
template void Table::each(std::function<void(uint32_t, const Lua::String&)>) const;
template void Table::each(std::function<void(uint32_t, const Lua::Boolean&)>) const;
template void Table::each(std::function<void(uint32_t, const Lua::Function&)>) const;
template void Table::each(std::function<void(uint32_t, const Lua::Table&)>) const;
template void Table::each(std::function<void(uint32_t, uint64_t* const&)>) const;

template<typename T>
void Table::try_get(const std::string& name, std::function<void(T&)> lambda)
{
    if (hasValue(name)) lambda(get<T>(name));
}
template void Table::try_get(const std::string&, std::function<void(Lua::Number&)>);
template void Table::try_get(const std::string&, std::function<void(Lua::String&)>);
template void Table::try_get(const std::string&, std::function<void(Lua::Boolean&)>);
template void Table::try_get(const std::string&, std::function<void(Lua::Function&)>);
template void Table::try_get(const std::string&, std::function<void(Lua::Table&)>);
template void Table::try_get(const std::string&, std::function<void(uint64_t*&)>);

template<typename T>
void Table::try_get(const std::string& name, std::function<void(const T&)> lambda) const
{
    if (hasValue(name)) lambda(get<T>(name));
}
template void Table::try_get(const std::string&, std::function<void(const Lua::Number&)>) const;
template void Table::try_get(const std::string&, std::function<void(const Lua::String&)>) const;
template void Table::try_get(const std::string&, std::function<void(const Lua::Boolean&)>) const;
template void Table::try_get(const std::string&, std::function<void(const Lua::Function&)>) const;
template void Table::try_get(const std::string&, std::function<void(const Lua::Table&)>) const;
template void Table::try_get(const std::string&, std::function<void(uint64_t* const&)>) const;

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

bool Table::hasValue(const std::string& name) const
{
    return dictionary.count(name);
}

template<typename T>
T& Table::get(const std::string& name)
{
    S2D_ASSERT(dictionary.count(name), "Dictionary doesn't have key");
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
    S2D_ASSERT(dictionary.count(name), "Dictionary doesn't have key");
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

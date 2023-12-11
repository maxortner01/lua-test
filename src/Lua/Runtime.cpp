#include <Simple2D/Lua/Runtime.hpp>

#include "Lua.cpp"

#include <iostream>

bool lua_check(lua_State* L, int r, std::optional<int> line = std::nullopt)
{
    if (r != LUA_OK)
    {
        std::string error_message = lua_tostring(L, -1);
        std::cout << "Message ";
        if (line.has_value()) std::cout << "from line " << line.value();
        std::cout << ": " << error_message << "\n";
        return false;
    }
    return true;
}

namespace S2D::Lua
{

Runtime::Runtime(const std::string& filename) :
    L(luaL_newstate()),
    _good(lua_check(STATE, luaL_dofile(STATE, filename.c_str()))),
    _filename(std::filesystem::path(filename).filename().c_str()),
    _last_modified(std::filesystem::last_write_time(std::filesystem::path(filename)))
{
    if (good()) luaL_openlibs(STATE);
}

Runtime::Runtime(Runtime&& r) :
    L(r.L),
    _good(r._good),
    _filename(r._filename)
{
    r.L = nullptr;
}

Runtime::~Runtime()
{
    if (L) lua_close(STATE);
    L = nullptr;
}

Runtime::Result<void>
Runtime::registerFunction(
    const std::string& table_name,
    const std::string& func_name,
    Lua::Function func)
{
    lua_getglobal(STATE, table_name.c_str());
    if (!lua_istable(STATE, -1))
    {
        lua_createtable(STATE, 0, 1);
        lua_setglobal(STATE, table_name.c_str());
        lua_getglobal(STATE, table_name.c_str());
        if (!lua_istable(STATE, -1)) return { ErrorCode::VariableDoesntExist };
    }
    lua_pushstring(STATE, func_name.c_str());
    lua_pushcfunction(STATE, reinterpret_cast<lua_CFunction>(func));
    lua_settable(STATE, -3);

    lua_pop(STATE, 1);

    return { };
}

template<typename T>
Runtime::Result<T>
Runtime::getGlobal(const std::string& name)
{
    lua_getglobal(STATE, name.c_str());
    const auto t = lua_type(STATE, -1);
    if (t != CompileTime::TypeMap<T>::LuaType)
        return { ErrorCode::TypeMismatch };
    
    return { CompileTime::TypeMap<T>::construct(L) };
}
template Runtime::Result<Number>   Runtime::getGlobal(const std::string&);
template Runtime::Result<String>   Runtime::getGlobal(const std::string&);
template Runtime::Result<Boolean>  Runtime::getGlobal(const std::string&);
template Runtime::Result<Table>    Runtime::getGlobal(const std::string&);

template<>
Runtime::Result<Lua::Function>
Runtime::getGlobal<Lua::Function>(const std::string& name)
{
    lua_getglobal(STATE, name.c_str());
    const auto t = lua_type(STATE, -1);
    if (t != LUA_TFUNCTION)
        return { ErrorCode::TypeMismatch };
    
    return { CompileTime::TypeMap<Lua::Function>::construct(L) };
}

template<typename T>
Runtime::Result<void>
Runtime::setGlobal(const std::string& name, const T& value)
{
    Lua::CompileTime::TypeMap<T>::push(L, value);
    lua_setglobal(STATE, name.c_str());
    return { };
}
template Runtime::Result<void> Runtime::setGlobal(const std::string&, const Lua::Number&);
template Runtime::Result<void> Runtime::setGlobal(const std::string&, const Lua::String&);
template Runtime::Result<void> Runtime::setGlobal(const std::string&, const Lua::Boolean&);
template Runtime::Result<void> Runtime::setGlobal(const std::string&, const Lua::Table&);
template Runtime::Result<void> Runtime::setGlobal(const std::string&, const Lua::Function&);

bool Runtime::good() const
{ return _good; }

Runtime::operator bool() const
{ return good(); }

void Runtime::_pop(std::size_t n) const
{
    lua_pop(STATE, n);
}

int Runtime::_call_func(uint32_t args, uint32_t ret) const
{
    return lua_pcall(STATE, args, ret, 0);
}

} // S2D::Lua

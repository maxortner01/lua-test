#include <Simple2D/Lua/Runtime.hpp>

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
    _good(lua_check(L, luaL_dofile(L, filename.c_str()))),
    _filename(std::filesystem::path(filename).filename().c_str()),
    _last_modified(std::filesystem::last_write_time(std::filesystem::path(filename)))
{
    if (good()) luaL_openlibs(L);
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
    if (L) lua_close(L);
    L = nullptr;
}

Runtime::Result<void>
Runtime::registerFunction(
    const std::string& table_name,
    const std::string& func_name,
    Lua::Function func)
{
    lua_getglobal(L, table_name.c_str());
    if (!lua_istable(L, -1))
    {
        lua_createtable(L, 0, 1);
        lua_setglobal(L, table_name.c_str());
        lua_getglobal(L, table_name.c_str());
        if (!lua_istable(L, -1)) return { ErrorCode::VariableDoesntExist };
    }
    lua_pushstring(L, func_name.c_str());
    lua_pushcfunction(L, func);
    lua_settable(L, -3);

    lua_pop(L, 1);

    return { };
}

template<typename T>
Runtime::Result<T>
Runtime::getGlobal(const std::string& name)
{
    lua_getglobal(L, name.c_str());
    if (!CompileTime::TypeMap<T>::check(L)) return { ErrorCode::TypeMismatch };
    return { CompileTime::TypeMap<T>::construct(L) };
}
template Runtime::Result<Number>   Runtime::getGlobal(const std::string&);
template Runtime::Result<String>   Runtime::getGlobal(const std::string&);
template Runtime::Result<Boolean>  Runtime::getGlobal(const std::string&);
template Runtime::Result<Function> Runtime::getGlobal(const std::string&);
template Runtime::Result<Table>    Runtime::getGlobal(const std::string&);

template<typename T>
Runtime::Result<void>
Runtime::setGlobal(const std::string& name, const T& value)
{
    Lua::CompileTime::TypeMap<T>::push(L, value);
    lua_setglobal(L, name.c_str());
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

} // S2D::Lua
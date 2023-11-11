#include <Simple2D/Lua/Runtime.hpp>

namespace S2D::Lua
{

Runtime::Runtime(const std::string& filename) :
    L(luaL_newstate()),
    _good(luaL_dofile(L, filename.c_str()) == LUA_OK)
{
    if (good()) luaL_openlibs(L);
}

Runtime::Runtime(Runtime&& r) :
    L(r.L),
    _good(r._good)
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

bool Runtime::good() const
{ return _good; }

Runtime::operator bool() const
{ return good(); }

} // S2D::Lua
#include <Simple2D/Lua/TypeMap.hpp>

namespace S2D::Lua
{
namespace CompileTime
{    
    int TypeMap<void*>::LuaType = LUA_TUSERDATA;
    template<> int TypeMap<Lua::Number>::LuaType = LUA_TNUMBER;
    template<> int TypeMap<Lua::String>::LuaType = LUA_TSTRING;
    template<> int TypeMap<Lua::Function>::LuaType = LUA_TFUNCTION;
    template<> int TypeMap<Lua::Boolean>::LuaType = LUA_TBOOLEAN;
    template<> int TypeMap<Lua::Table>::LuaType = LUA_TTABLE;
    
    bool
    TypeMap<void*>::check(State L)
    {
        return lua_isuserdata(reinterpret_cast<lua_State*>(L), -1);
    }

    void 
    TypeMap<void*>::push(State L, void* val)
    {
        auto* ptr = lua_newuserdata(reinterpret_cast<lua_State*>(L), sizeof(void*));
        *reinterpret_cast<void**>(ptr) = val;
    }

    void* 
    TypeMap<void*>::construct(State L)
    {
        return *reinterpret_cast<void**>(lua_touserdata(reinterpret_cast<lua_State*>(L), -1));
    }

    template<>
    bool 
    TypeMap<Lua::Number>::check(State L)
    {
        return lua_isnumber(reinterpret_cast<lua_State*>(L), -1);
    }

    template<>
    void
    TypeMap<Lua::Number>::push(State L, const Lua::Number& number)
    {
        lua_pushnumber(reinterpret_cast<lua_State*>(L), number);
    }

    template<>
    Lua::Number 
    TypeMap<Lua::Number>::construct(State L)
    {
        return static_cast<Lua::Number>(lua_tonumber(reinterpret_cast<lua_State*>(L), -1));
    }


    template<>
    bool 
    TypeMap<Lua::String>::check(State L)
    {
        return lua_isstring(reinterpret_cast<lua_State*>(L), -1);
    }
    
    template<>
    void
    TypeMap<Lua::String>::push(State L, const Lua::String& string)
    {
        lua_pushstring(reinterpret_cast<lua_State*>(L), string.c_str());
    }

    template<>
    Lua::String 
    TypeMap<Lua::String>::construct(State L)
    {
        return Lua::String(lua_tostring(reinterpret_cast<lua_State*>(L), -1));
    }
    

    template<>
    bool
    TypeMap<Lua::Function>::check(State L)
    {
        return lua_iscfunction(reinterpret_cast<lua_State*>(L), -1);
    }

    template<>
    void
    TypeMap<Lua::Function>::push(State L, const Lua::Function& function)
    {
        lua_pushcfunction(reinterpret_cast<lua_State*>(L), reinterpret_cast<lua_CFunction>(function));
    }

    template<>
    Lua::Function
    TypeMap<Lua::Function>::construct(State L)
    {
        return (Lua::Function)lua_tocfunction(reinterpret_cast<lua_State*>(L), -1);
    }


    template<>
    bool 
    TypeMap<Lua::Boolean>::check(State L)
    {
        return lua_isboolean(reinterpret_cast<lua_State*>(L), -1);
    }

    template<>
    void
    TypeMap<Lua::Boolean>::push(State L, const Lua::Boolean& boolean)
    {
        lua_pushboolean(reinterpret_cast<lua_State*>(L), boolean);
    }

    template<>
    Lua::Boolean 
    TypeMap<Lua::Boolean>::construct(State L)
    {
        return lua_toboolean(reinterpret_cast<lua_State*>(L), -1);
    }

    
    template<>
    void
    TypeMap<Lua::Table>::push(State L, const Table& val)
    {
        val.toStack(L);
    }

    template<>
    Lua::Table
    TypeMap<Lua::Table>::construct(State L)
    {
        return Lua::Table(L);
    }
    
    template<>
    bool
    TypeMap<Lua::Table>::check(State L)
    {
        return lua_istable(reinterpret_cast<lua_State*>(L), -1);
    }

}
}

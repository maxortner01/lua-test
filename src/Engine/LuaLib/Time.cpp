#include <Simple2D/Engine/LuaLib/Time.hpp>
#include <Simple2D/Engine/Core.hpp>
#include <Simple2D/Def.hpp>

#include "../../Lua/Lua.cpp"

namespace S2D::Engine
{

int Time::deltaTime(Lua::State L)
{
    S2D_ASSERT(!lua_gettop(STATE), "Lua argument size mismatch");
    lua_pushnumber(STATE, (Lua::Number)dt);
    return 1;
}

Time::Time() : Base("Time",
    {
        { "deltaTime", Time::deltaTime }
    })
{   }

}

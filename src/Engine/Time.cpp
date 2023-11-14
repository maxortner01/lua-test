#include <Simple2D/Engine/Time.hpp>
#include <Simple2D/Engine/Core.hpp>

namespace S2D::Engine
{

int Time::deltaTime(Lua::State L)
{
    assert(!lua_gettop(L));
    const auto dt = Core::get().getDeltaTime();
    lua_pushnumber(L, (Lua::Number)dt);
    return 1;
}

Time::Time() : Base("Time",
    {
        { "deltaTime", Time::deltaTime }
    })
{   }

}
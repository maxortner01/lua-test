#include <Simple2D/Engine/LuaLib/Input.hpp>

namespace S2D::Engine
{
    int Input::getMousePosition(Lua::State L)
    {
        S2D_ASSERT(lua_gettop(L) == 0, "This method doesn't take arguments");
        Lua::Table mouse_pos;
        mouse_pos.set<Lua::Number>("x", Input::mouse_position.x);
        mouse_pos.set<Lua::Number>("y", Input::mouse_position.y);
        mouse_pos.toStack(L);

        return 1;
    }

    int Input::getPressed(Lua::State L)
    {
        const auto [ key_code ] = extractArgs<Lua::String>(L);
        if (!global_state.count(key_code)) lua_pushboolean(L, false);
        else lua_pushboolean(L, (int)global_state.at(key_code) & (int)(KeyState::Press));
        return 1;
    }

    int Input::getDown(Lua::State L)
    {
        const auto [ key_code ] = extractArgs<Lua::String>(L);
        if (!global_state.count(key_code)) lua_pushboolean(L, false);
        else lua_pushboolean(L, (int)global_state.at(key_code) & (int)(KeyState::Down));
        return 1;
    }

    int Input::getReleased(Lua::State L)
    {
        const auto [ key_code ] = extractArgs<Lua::String>(L);
        if (!global_state.count(key_code)) lua_pushboolean(L, false);
        else lua_pushboolean(L, (int)global_state.at(key_code) & (int)(KeyState::Release));
        return 1;
    }

    Input::Input() : Base("Input",
        {
            { "getPressed",  Input::getPressed  },
            { "getDown",     Input::getDown     },
            { "getReleased", Input::getReleased },
            { "getMousePosition", Input::getMousePosition }
        })
    {   }
}
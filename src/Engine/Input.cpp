#include <Simple2D/Engine/Input.hpp>

namespace S2D::Engine
{
    int Input::getPressed(Lua::State L)
    {
        const auto [ key_code ] = extractArgs<Lua::String>(L);
        S2D_ASSERT(key_code.size() == 1, "Must be key character");
        if (!global_state.count(key_code[0])) lua_pushboolean(L, false);
        else lua_pushboolean(L, (int)global_state.at(key_code[0]) & (int)(KeyState::Press));
        return 1;
    }

    int Input::getDown(Lua::State L)
    {
        const auto [ key_code ] = extractArgs<Lua::String>(L);
        S2D_ASSERT(key_code.size() == 1, "Must be key character");
        if (!global_state.count(key_code[0])) lua_pushboolean(L, false);
        else lua_pushboolean(L, (int)global_state.at(key_code[0]) & (int)(KeyState::Down));
        return 1;
    }

    int Input::getReleased(Lua::State L)
    {
        const auto [ key_code ] = extractArgs<Lua::String>(L);
        S2D_ASSERT(key_code.size() == 1, "Must be key character");
        if (!global_state.count(key_code[0])) lua_pushboolean(L, false);
        else lua_pushboolean(L, (int)global_state.at(key_code[0]) & (int)(KeyState::Release));
        return 1;
    }

    Input::Input() : Base("Input",
        {
            { "getPressed",  Input::getPressed  },
            { "getDown",     Input::getDown     },
            { "getReleased", Input::getReleased }
        })
    {   }
}
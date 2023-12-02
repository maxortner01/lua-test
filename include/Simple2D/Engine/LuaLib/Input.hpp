#pragma once

#include "../../Lua.hpp"
#include "../../Util/Vector.hpp"

namespace S2D::Engine
{
    struct Input : Lua::Lib::Base
    {
        enum class KeyState
        {
            Press   = 0b001,
            Down    = 0b010,
            Release = 0b100
        };

        enum class Button
        {
            W, A, S, D,
            LeftClick, RightClick
        };  

        inline static std::unordered_map<std::string, KeyState> global_state;
        inline static Math::Vec2f mouse_position; // World position

        static int getMousePosition(Lua::State L);
        static int getPressed(Lua::State L);
        static int getDown(Lua::State L);
        static int getReleased(Lua::State L);
        Input();
    };

    #define KEY(name) case Input::Button::name: return #name

    inline static const char* operator*(Input::Button b)
    {
        switch (b)
        {
        KEY(W); KEY(A); KEY(S); KEY(D);
        KEY(LeftClick); KEY(RightClick);
        }
    }

    #undef KEY
}
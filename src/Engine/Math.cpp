#include <Simple2D/Engine/Math.hpp>

namespace S2D::Engine
{
    int Math::normalize(Lua::State L)
    {
        const auto [ vector ] = extractArgs<Lua::Table>(L);
        float length = 0.f;
        if (vector.hasValue("x")) length += pow(vector.get<Lua::Number>("x"), 2);
        if (vector.hasValue("y")) length += pow(vector.get<Lua::Number>("y"), 2);
        if (vector.hasValue("z")) length += pow(vector.get<Lua::Number>("z"), 2);
        length = sqrt(length);

        if (length == 0)
        {
            Lua::Table ret;
            if (vector.hasValue("x")) ret.set<Lua::Number>("x", 0);
            if (vector.hasValue("y")) ret.set<Lua::Number>("y", 0);
            if (vector.hasValue("z")) ret.set<Lua::Number>("z", 0);
            ret.toStack(L);
            return 1;
        }

        Lua::Table ret;
        if (vector.hasValue("x")) ret.set<Lua::Number>("x", vector.get<Lua::Number>("x") / length);
        if (vector.hasValue("y")) ret.set<Lua::Number>("y", vector.get<Lua::Number>("y") / length);
        if (vector.hasValue("z")) ret.set<Lua::Number>("z", vector.get<Lua::Number>("z") / length);
        ret.toStack(L);
        return 1;
    }

    Math::Math() : Base("Math",
        {
            { "normalize", Math::normalize }    
        })
    {   }
}
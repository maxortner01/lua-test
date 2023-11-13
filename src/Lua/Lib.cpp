#include <Simple2D/Lua/Lib.hpp>
#include <Simple2D/Lua/Runtime.hpp>

namespace S2D::Lua::Lib
{

void Base::registerFunctions(Lua::Runtime& runtime) const
{
    for (auto& p : _funcs)
        runtime.registerFunction(_name, p.first, p.second);
}

Lua::Table Base::asTable() const
{
    Lua::Table table;

    for (auto& p : _funcs)
        table.set(p.first, p.second);

    return table;
}

Base::Base(
    const std::string& name,
    const Base::Map& funcs) :
        _name(name),
        _funcs(funcs)
{   }

} // S2D::Lua::Lib
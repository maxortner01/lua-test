#include <Simple2D/Lua/Lib.hpp>

namespace S2D::Lua::Lib
{

void Base::registerFunctions(Lua::Runtime& runtime)
{
    for (auto& p : _funcs)
        runtime.registerFunction(_name, p.first, p.second);
}

Base::Base(
    const std::string& name,
    const Base::Map& funcs) :
        _name(name),
        _funcs(funcs)
{   }

} // S2D::Lua::Lib
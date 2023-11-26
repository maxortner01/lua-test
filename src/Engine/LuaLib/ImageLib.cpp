#include <Simple2D/Engine/LuaLib/ImageLib.hpp>

#include <Simple2D/Log/Log.hpp>

#include <SFML/Graphics.hpp>

#define LUA_EXCEPTION(expr, msg) if (!(expr)) { logger->error(msg); return 0; }

namespace S2D::Engine
{

int 
ImageLib::getSize(Lua::State L)
{
    auto& logger = Log::Logger::instance("engine");
    const auto [ resource ] = extractArgs<Lua::Table>(L);
    LUA_EXCEPTION(resource.hasValue("resource"), "Resource missing reference");

    auto* image = (sf::Image*)*resource.get<uint64_t*>("resource");

    const auto size = image->getSize();
    
    Lua::Table s;
    s.set("width",  (Lua::Number)size.x);
    s.set("height", (Lua::Number)size.y);
    s.toStack(L);

    return 1;
}

int
ImageLib::getPixel(Lua::State L)
{
    auto& logger = Log::Logger::instance("engine");
    const auto [ resource, x, y ] = extractArgs<Lua::Table, Lua::Number, Lua::Number>(L);
    LUA_EXCEPTION(resource.hasValue("resource"), "Resource missing reference");
    LUA_EXCEPTION(x >= 0 && y >= 0, "Coordinates can not be negative");

    auto* image = (sf::Image*)*resource.get<uint64_t*>("resource");
    const auto pixel = image->getPixel(sf::Vector2u(x, y));

    Lua::Table p;
    p.set("r", (Lua::Number)pixel.r);
    p.set("g", (Lua::Number)pixel.g);
    p.set("b", (Lua::Number)pixel.b);
    p.set("a", (Lua::Number)pixel.a);
    p.toStack(L);

    return 1;
}

ImageLib::ImageLib() : Base("Image",
    {
        { "getPixel", ImageLib::getPixel },
        { "getSize",  ImageLib::getSize  }
    })
{   }

}
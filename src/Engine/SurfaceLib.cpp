#include <Simple2D/Engine/SurfaceLib.hpp>
#include <Simple2D/Engine/Core.hpp>

#include <Simple2D/Log/Log.hpp>

#define LUA_ASSERT(expr, msg) if (!(expr)) { Log::Logger::instance("lua")->error(msg); return 0; }

namespace S2D::Engine
{
    int SurfaceLib::drawText(Lua::State L)
    {
        const auto [ surface_table, x, y, size, text, font ] = 
            extractArgs<Lua::Table, Lua::Number, Lua::Number, Lua::Number, Lua::String, Lua::String>(L);


        LUA_ASSERT(surface_table.hasValue("scene"), "Surface missing scene");
        const auto* scene = (Scene*)*surface_table.get<uint64_t*>("scene");

        LUA_ASSERT(surface_table.hasValue("surface"), "Surface missing render surface");
        auto* surface = (sf::RenderTexture*)*surface_table.get<uint64_t*>("surface");

        const auto font_r = scene->resources.getResource<sf::Font>(font);
        LUA_ASSERT(font_r, "Scene missing font '" + font + "'");
        const auto* sf_font = font_r.value();

        sf::Text t(*sf_font, text);
        t.setCharacterSize(size);
        t.setPosition(sf::Vector2f((int)x, (int)y));
        surface->draw(t);

        return 0;
    }
    
    SurfaceLib::SurfaceLib() : Base("Surface",
        {
            { "drawText", SurfaceLib::drawText }    
        })
    {   }
}
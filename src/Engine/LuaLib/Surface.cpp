#include <Simple2D/Engine/LuaLib/Surface.hpp>
#include <Simple2D/Engine/LuaLib/ResLib.hpp>
#include <Simple2D/Engine/Core.hpp>

#include <Simple2D/Log/Log.hpp>

#define LUA_ASSERT(expr, msg) if (!(expr)) { Log::Logger::instance("lua")->error(msg); return 0; }

namespace S2D::Engine
{
    int Surface::drawText(Lua::State L)
    {
        //S2D_ASSERT(false, "Text not supported");

        const auto [ surface_table, x, y, size, text, font ] = 
            extractArgs<Lua::Table, Lua::Number, Lua::Number, Lua::Number, Lua::String, Lua::String>(L);

        LUA_ASSERT(surface_table.hasValue("scene"), "Surface missing scene");
        auto* scene = (Scene*)*surface_table.get<uint64_t*>("scene");

        LUA_ASSERT(surface_table.hasValue("surface"), "Surface missing render surface");
        auto* surface = (Graphics::DrawTexture*)*surface_table.get<uint64_t*>("surface");

        const auto font_r = scene->resources.getResource<Graphics::Font>(font);
        LUA_ASSERT(font_r, "Scene missing font '" + font + "'");
        auto* graphics_font = font_r.value();

        Math::Transform transform;
        transform.translate(Math::Vec3f(x, y, 0));

        scene->renderer->renderText(transform, *surface, text, graphics_font, (uint32_t)size);

        return 0;
    }
    
    // Takes in the surface, name, type, transform info
    int Surface::drawTexture(Lua::State L)
    {
        const auto [ surface_table, name, type, transform_table ] = 
            extractArgs<Lua::Table, Lua::String, Lua::Number, Lua::Table>(L);

        LUA_ASSERT(surface_table.hasValue("scene"), "Surface missing scene");
        const auto* scene = (Scene*)*surface_table.get<uint64_t*>("scene");

        LUA_ASSERT(surface_table.hasValue("surface"), "Surface missing render surface");
        auto* surface = (Graphics::DrawTexture*)*surface_table.get<uint64_t*>("surface");

        // Get the resource
        const auto res_type = (ResourceType)(int)type;
        const Graphics::Texture* texture = nullptr;
        switch(res_type)
        {
        case ResourceType::Texture:
        {
            const auto res = scene->resources.getResource<Graphics::Texture>(name);
            LUA_ASSERT(res, "Error loading texture");
            texture = res.value();
            break;
        }
        case ResourceType::Surface:
        {
            LUA_ASSERT(scene->renderpass->targets.count(name), "Renderpass missing requested target");
            texture = scene->renderpass->targets.at(name)->texture();
            break;
        }
        default: LUA_ASSERT(res_type == ResourceType::Texture || res_type == ResourceType::Surface, "Can only draw texture of texture or surface resource");
        }

        // Get the transform info
        Math::Vec2f position, scale;
        float rotation = 0.f;

        transform_table.try_get<Lua::Table>("position", 
            [&](const Lua::Table& table)
            {
                position.x = ( table.hasValue("x") ? table.get<Lua::Number>("x") : 0.f );
                position.y = ( table.hasValue("y") ? table.get<Lua::Number>("y") : 0.f );
            });

        transform_table.try_get<Lua::Number>("scale",
            [&](const Lua::Number& s)
            { scale.x = s; scale.y = s; });

        transform_table.try_get<Lua::Number>("rotation", [&](const Lua::Number& r) { rotation = r; });

        LUA_ASSERT(texture, "Error getting texture");

        S2D_ASSERT(false, "Sprite drawing not supported");
        /*
        sf::Sprite sprite(*texture);
        sprite.move(position);
        sprite.scale(scale);
        sprite.rotate(sf::degrees(rotation));
        surface->draw(sprite);*/

        return 0;
    }

    
    Surface::Surface() : Base("Surface",
        {
            { "drawText",    Surface::drawText    },
            { "drawTexture", Surface::drawTexture }  
        })
    {   }
}
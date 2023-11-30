#include <Simple2D/Engine/LuaLib/ResLib.hpp>
#include <Simple2D/Engine/LuaLib/Entity.hpp>
#include <Simple2D/Engine/LuaLib/ImageLib.hpp>

#include <Simple2D/Engine/Core.hpp>

#include <Simple2D/Log/Log.hpp>

#define LUA_EXCEPTION(expr, msg) if (!(expr)) { logger->error(msg); return 0; }

namespace S2D::Engine
{

const char* operator*(ResourceType r)
{
    switch (r)
    {
    case ResourceType::Font:    return "Font";
    case ResourceType::Image:   return "Image";
    case ResourceType::Texture: return "Texture";
    case ResourceType::Surface: return "Surface";
    default: return "";
    }
}

int
ResLib::getResource(Lua::State L)
{
    auto& logger = Log::Logger::instance("engine");
    const auto [ world_table, type, name ] 
        = extractArgs<Lua::Table, Lua::Number, Lua::String>(L);
    
    LUA_EXCEPTION(world_table.hasValue("scene"), "World missing scene instance");
    auto* scene = (Scene*)*world_table.get<uint64_t*>("scene");

    LUA_EXCEPTION(type >= 0 && type <= (Lua::Number)(int)ResourceType::Image, "Resource type invalid!");

    void* resource = nullptr;
    const auto res_type = (ResourceType)(int)type;
    switch (res_type)
    {
    case ResourceType::Font:
    {
        const auto res = scene->resources.getResource<sf::Font>(name);
        LUA_EXCEPTION(res, "Resource not found!");
        resource = (void*)res.value();

        return 0;
    }
    case ResourceType::Image:
    {
        const auto res = scene->resources.getResource<sf::Image>(name);
        LUA_EXCEPTION(res, "Resource not found!");
        resource = (void*)res.value();

        auto table = ImageLib().asTable();
        table.set("resource", (uint64_t)resource);
        table.toStack(L);
        return 1;
    }
    case ResourceType::Texture:
    {
        const auto res = scene->resources.getResource<sf::Texture>(name);
        LUA_EXCEPTION(res, "Resource not found!");
        resource = (void*)res.value();
        
        return 0;
    }
    case ResourceType::Surface:
    {
        LUA_EXCEPTION(scene->renderpass->targets.count(name), "Renderpass missing requested surface");
        auto& res = scene->renderpass->targets.at(name);
        resource = (void*)res.get();

        return 0;
    }
    default: return 0;
    }
}

ResLib::ResLib() : Base("Resources",
    {
        { "getResource", ResLib::getResource }    
    })
{   }

}
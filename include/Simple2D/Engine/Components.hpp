#pragma once

#include "../Lua.hpp"

#include <flecs.h>
#include <SFML/Graphics.hpp>

namespace S2D::Engine
{
    /**
     * @brief Assigns component world IDs to their respective name key in the table
     * @param table Table to set the names into
     */
    void
    registerComponents(Lua::Table& table, flecs::world& world);

    struct Script
    {
        std::unique_ptr<Lua::Runtime> runtime;
        bool initialized;
    };

    Script loadScript(const std::string& filename, flecs::world& world);

    enum class Name
    {
        Transform,
        Rigidbody,
        Sprite,
        Text,
        Tilemap,
        Count
    };

    struct Dead { };

    static const char* operator*(Name name)
    {
        switch (name)
        {
        case Name::Transform: return "Transform";
        case Name::Rigidbody: return "Rigidbody";
        case Name::Sprite:    return "Sprite";
        case Name::Text:      return "Text";
        case Name::Tilemap:   return "Tilemap";
        default: return "";
        }
    }

    template<Name T>
    struct Component;

    template<>
    struct Component<Name::Transform>
    {
        static constexpr Name Type = Name::Transform;
        struct Data
        {
            sf::Vector3f position;
            float rotation;
        };

        static Lua::Table getTable(const Data& data);
        static void fromTable(const Lua::Table& table, void* _data);
    };

    template<>
    struct Component<Name::Rigidbody>
    {
        static constexpr Name Type = Name::Rigidbody;
        struct Data
        {
            struct {
                float x, y;
            } velocity;
        };

        static Lua::Table getTable(const Data& data);
        static void fromTable(const Lua::Table& table, void* _data);
    };

    template<>
    struct Component<Name::Sprite>
    {
        static constexpr Name Type = Name::Sprite;
        struct Data
        {
            Lua::String texture;
            sf::Vector2u size;
        };

        static Lua::Table getTable(const Data& data);
        static void fromTable(const Lua::Table& table, void* _data);
    };

    template<>
    struct Component<Name::Text>
    {
        static constexpr Name Type = Name::Text;
        struct Data
        {
            Lua::String string;
            Lua::String font;
            Lua::Number character_size;
        };

        static Lua::Table getTable(const Data& data);
        static void fromTable(const Lua::Table& table, void* _data);
    };

    template<>
    struct Component<Name::Tilemap>
    {
        struct Tile
        {
            sf::Vector2u texture_coords;
        };

        struct Map
        {
            std::unordered_map<int32_t, Tile> map;

            void setTile(int16_t x, int16_t y, const Tile& tile);
        };

        static constexpr Name Type = Name::Tilemap;
        struct Data
        {
            Map tiles;
            sf::Vector2f tilesize;
            struct 
            {
                std::string texture_name;
            } spritesheet;
        };

        static int setTile(Lua::State L);
        static Lua::Table getTable(const Data& data);
        static void fromTable(const Lua::Table& table, void* _data);
    };

    template<Name T>
    using ComponentData = typename Component<T>::Data;
}

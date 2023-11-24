#pragma once

#include "../Lua.hpp"

#include "Mesh.hpp"

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
        std::vector<std::pair<std::shared_ptr<Lua::Runtime>, int>> runtime;
        inline static std::string SourceDir;
    };

    void loadScript(const std::string& filename, flecs::world& world, Script& script);

    enum class Name
    {
        Transform,
        Rigidbody,
        Sprite,
        Text,
        Tilemap,
        Collider,
        Camera,
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
        case Name::Collider:  return "Collider";
        case Name::Camera:    return "Camera";
        default: return "";
        }
    }

    void
    setComponentFromTable(
        const Lua::Table& table, 
        void* _data, 
        flecs::id_t component_id, 
        flecs::world& world);

    template<Name T>
    struct Component;

    template<>
    struct Component<Name::Transform>
    {
        static constexpr Name Type = Name::Transform;
        struct Data
        {
            sf::Vector3f position;
            float scale = 1.f;
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
            Lua::Number linear_drag;
            sf::Vector2f added_force;
            sf::Vector2f velocity;
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
            std::shared_ptr<RawMesh> mesh;
            Lua::String texture;
            sf::Vector2u size;
        };

        static Lua::Table getTable(const Data& data);
        static void fromTable(const Lua::Table& table, void* _data);
    };

    enum class TextAlign
    {
        Left, Center, Right
    };

    template<>
    struct Component<Name::Text>
    {
        static constexpr Name Type = Name::Text;
        struct Data
        {
            TextAlign   align;
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

        enum class LayerState
        {
            Solid, NotSolid
        };

        struct Map
        {
            std::unordered_map<
                uint32_t, // Layer number
                std::pair<std::unordered_map<int32_t, Tile>, LayerState> // Layer
            > map;
            bool changed;

            void setTile(int16_t x, int16_t y, uint32_t layer, const Tile& tile);
            void setLayerState(uint32_t layer, LayerState state);
        };

        static constexpr Name Type = Name::Tilemap;
        struct Data
        {
            std::unique_ptr<RawMesh> mesh;
            Map tiles;
            sf::Vector2f tilesize;
            struct 
            {
                std::string texture_name;
            } spritesheet;
        };

        static int setTile(Lua::State L);
        static int setLayerState(Lua::State L);

        static Lua::Table getTable(const Data& data);
        static void fromTable(const Lua::Table& table, void* _data);
    };

    template<>
    struct Component<Name::Collider>
    {
        static constexpr Name Type = Name::Collider;
        struct Data
        {
            Lua::Number collider_component;
            std::unique_ptr<CollisionMesh> mesh;
        };

        static Lua::Table getTable(const Data& data);
        static void fromTable(const Lua::Table& table, void* _data);
    };  

    template<>
    struct Component<Name::Camera>
    {
        static constexpr Name Type = Name::Camera;
        struct Data
        {
            Lua::Number FOV;
            enum class Projection
            {
                Orthographic, Perspective
            } projection;
        };

        static Lua::Table getTable(const Data& data);
        static void fromTable(const Lua::Table& table, void* _data);
    };

    template<Name T>
    using ComponentData = typename Component<T>::Data;

    using Transform = ComponentData<Name::Transform>;
    using Tilemap   = ComponentData<Name::Tilemap>;
    using Text      = ComponentData<Name::Text>;
    using Sprite    = ComponentData<Name::Sprite>;
    using Collider  = ComponentData<Name::Collider>;
    using Rigidbody = ComponentData<Name::Rigidbody>;
    using Camera    = ComponentData<Name::Camera>;    
}

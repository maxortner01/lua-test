#pragma once

#include "../Lua.hpp"

#include "Mesh.hpp"

#include <flecs.h>
#include <SFML/Graphics.hpp>

#define COMPONENT_DEFINITION(name, contents)                            \
    template<> struct Component<Name::name>                             \
    {                                                                   \
        static constexpr Name Type = Name::name;                        \
        struct Data                                                     \
        {                                                               \
            contents                                                    \
        };                                                              \
        static Lua::Table getTable(const Data& data);                   \
        static void fromTable(const Lua::Table& table, void* _data);    \
    }   

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

    // Assumes the _Enum has a ::Count member and that there is a *operator overload
    // That gives the name
    template<typename _Enum>
    void globalEnum(Lua::Runtime& runtime, const std::string& name)
    {
        using namespace Util::CompileTime;

        Lua::Table table;
        for (uint32_t i = 0; i < (int)_Enum::Count; i++)
            table.set(*(_Enum)i, (Lua::Number)(int)i);

        runtime.setGlobal(name, table);
    }

    std::unique_ptr<Lua::Runtime>
    loadRuntime(const std::string& filename, flecs::world& world);

    void 
    loadScript(const std::string& filename, flecs::world& world, Script& script);

    enum class Name
    {
        Transform,
        Rigidbody,
        Sprite,
        Text,
        Tilemap,
        Collider,
        Camera,
        CustomMesh,
        Shader,
        Count
    };

    struct Dead { };

    static const char* operator*(Name name)
    {
        switch (name)
        {
        case Name::Transform:  return "Transform";
        case Name::Rigidbody:  return "Rigidbody";
        case Name::Sprite:     return "Sprite";
        case Name::Text:       return "Text";
        case Name::Tilemap:    return "Tilemap";
        case Name::Collider:   return "Collider";
        case Name::Camera:     return "Camera";
        case Name::CustomMesh: return "CustomMesh";
        case Name::Shader:     return "Shader";
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

    COMPONENT_DEFINITION(Transform,
        sf::Vector3f position;
        float scale = 1.f;
        float rotation;
    );

    COMPONENT_DEFINITION(Rigidbody,
        Lua::Number linear_drag;
        sf::Vector2f added_force;
        sf::Vector2f velocity;
    );

    COMPONENT_DEFINITION(Sprite,
        std::shared_ptr<RawMesh> mesh;
        Lua::String texture;
        sf::Vector2f size;
    );

    enum class TextAlign
    {
        Left, Center, Right
    };

    COMPONENT_DEFINITION(Text,
        TextAlign   align;
        Lua::String string;
        Lua::String font;
        Lua::Number character_size;
    );

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

    COMPONENT_DEFINITION(Collider,
        Lua::Number collider_component;
        std::unique_ptr<CollisionMesh> mesh;    
    );

    enum class Projection
    {
        Orthographic, Perspective, Count
    };

    const char* operator*(Projection p);

    sf::Vector2f toScreenSpace(flecs::entity camera, const sf::Vector2f& world_point);
    sf::Vector2f toWorldSpace(flecs::entity camera, const sf::Vector2f& screen_point);

    COMPONENT_DEFINITION(Camera,
        Lua::Number FOV;
        Projection projection;
        sf::Vector2u size;
    );

    COMPONENT_DEFINITION(CustomMesh,
        std::unique_ptr<RawMesh> mesh;
    );

    // If the resource we reference from a shader is in the current renderpass
    // or if its in the scene's resource
    enum class TexSource
    {
        Resources, Renderpass, Count
    };

    using ResList = std::vector<std::pair<TexSource, Lua::String>>;
    COMPONENT_DEFINITION(Shader,
        Lua::String name;
        ResList textures;
    );

    template<Name T>
    using ComponentData = typename Component<T>::Data;

    using Transform  = ComponentData<Name::Transform>;
    using Tilemap    = ComponentData<Name::Tilemap>;
    using Text       = ComponentData<Name::Text>;
    using Sprite     = ComponentData<Name::Sprite>;
    using Collider   = ComponentData<Name::Collider>;
    using Rigidbody  = ComponentData<Name::Rigidbody>;
    using Camera     = ComponentData<Name::Camera>;    
    using CustomMesh = ComponentData<Name::CustomMesh>;    
    using ShaderComp = ComponentData<Name::Shader>;
}

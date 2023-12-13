#include <Simple2D/Engine/Components.hpp>

#include <Simple2D/Engine/LuaLib/Time.hpp>
#include <Simple2D/Engine/LuaLib/Entity.hpp>
#include <Simple2D/Engine/LuaLib/Input.hpp>
#include <Simple2D/Engine/LuaLib/Math.hpp>
#include <Simple2D/Engine/LuaLib/MeshLib.hpp>
#include <Simple2D/Engine/LuaLib/ResLib.hpp>

#include <Simple2D/Log/Library.hpp>
#include <Simple2D/Log/Log.hpp>

#include <Simple2D/Util.hpp>

#include <flecs.h>

namespace S2D::Engine
{

std::unique_ptr<Lua::Runtime>
loadRuntime(const std::string& filename, flecs::world& world)
{
    return std::make_unique<Lua::Runtime>([&]()
    {
        auto runtime = Lua::Runtime::create<
            Log::Library, Time, Engine::Input, Engine::Math
        >(filename);

        /* The component name enum */
        Lua::Table component;
        registerComponents(component, world);
        runtime.setGlobal("Component", component);

        /* The layer state enum */
        Lua::Table LayerState;
        LayerState.set<Lua::Number>("Solid",    (int)Component<Name::Tilemap>::LayerState::Solid);
        LayerState.set<Lua::Number>("NotSolid", (int)Component<Name::Tilemap>::LayerState::NotSolid);
        runtime.setGlobal("LayerState", LayerState);

        /* Directory */
        Lua::Table Directory;
        Directory.set<Lua::String>("Source", Script::SourceDir);
        runtime.setGlobal("Directory", Directory);

        globalEnum<Primitive>   (runtime, "PrimitiveType");
        globalEnum<ResourceType>(runtime, "ResourceType");
        globalEnum<Projection>  (runtime, "ProjectionType");

        return runtime;
    }());
}

void loadScript(const std::string& filename, flecs::world& world, Script& script)
{
    script.runtime.push_back(std::pair(
        loadRuntime(filename, world), 
        false
    ));
}

void
setComponentFromTable(
    const Lua::Table& table, 
    void* _data, 
    flecs::id_t component_id, 
    flecs::world& world)
{
    using namespace Util::CompileTime;

    bool found = false;
    static_for<(int)Name::Count>([&](auto n)
    {
        if (found) return;

        const std::size_t i = n;
        const auto actual_id = world.component<ComponentData<(Name)i>>().raw_id();
        if (actual_id == component_id)
        {
            found = true;
            Component<(Name)i>::fromTable(table, _data);
        }
    });
    
    S2D_ASSERT(found, "Component doesn't exist");
}

/* Position */
Lua::Table 
Component<Name::Transform>::getTable(
    const Data& data)
{
    Lua::Table table;

    Lua::Table position;
    position.set("x", data.position.x);
    position.set("y", data.position.y);
    position.set("z", data.position.z);
    table.set("position", position);
    table.set("rotation", data.rotation);
    table.set("scale", data.scale);

    return table;
}

void
Component<Name::Transform>::fromTable(
    const Lua::Table& table,
    void* _data)
{
    auto* data = reinterpret_cast<Data*>(_data);

    const auto& position = table.get<Lua::Table>("position");
    data->position.x = position.get<float>("x");
    data->position.y = position.get<float>("y");
    data->position.z = position.get<float>("z");
    data->rotation = table.get<Lua::Number>("rotation");
    data->scale = table.get<Lua::Number>("scale");
}

S2D::Math::Transform modelTransform(const Transform* transform)
{
    S2D::Math::Transform model;
    model.translate(transform->position);
    model.scale({ transform->scale, transform->scale, transform->scale });
    model.rotate({ 0.f, 0.f, transform->rotation });
    return model;
}

/* Rigidbody */
Lua::Table 
Component<Name::Rigidbody>::getTable(
    const ComponentData<Name::Rigidbody>& data)
{
    Lua::Table table;
    
    Lua::Table velocity;
    velocity.set("x", data.velocity.x);
    velocity.set("y", data.velocity.y);
    velocity.set("z", data.velocity.z);
    table.set("velocity", velocity);

    Lua::Table added_force;
    added_force.set("x", data.added_force.x);
    added_force.set("y", data.added_force.y);
    added_force.set("z", data.added_force.z);
    table.set("addedForce", added_force);

    table.set("linearDrag", data.linear_drag);
    return table;
}

void
Component<Name::Rigidbody>::fromTable(
    const Lua::Table& table,
    void* _data)
{
    auto* data = reinterpret_cast<Data*>(_data);
    const auto& velocity = table.get<Lua::Table>("velocity");
    data->velocity.x = velocity.get<float>("x");
    data->velocity.y = velocity.get<float>("y");
    data->velocity.z = velocity.get<float>("z");

    const auto& added_force = table.get<Lua::Table>("addedForce");
    data->added_force.x = added_force.get<Lua::Number>("x");
    data->added_force.y = added_force.get<Lua::Number>("y");
    data->added_force.z = added_force.get<Lua::Number>("z");

    data->linear_drag = table.get<Lua::Number>("linearDrag");
}

Lua::Table 
Component<Name::Sprite>::getTable(
    const Data& data)
{
    Lua::Table table;

    Lua::Table size;
    size.set("width",  data.size.x);
    size.set("height", data.size.y);
    table.set("size", size);
    table.set("texture", data.texture);

    return table;
}

void 
Component<Name::Sprite>::fromTable(
    const Lua::Table& table, 
    void* _data)
{
    auto* data = reinterpret_cast<Data*>(_data);
    const auto& size = table.get<Lua::Table>("size");
    data->size.x = size.get<Lua::Number>("width");
    data->size.y = size.get<Lua::Number>("height");
    data->texture = table.get<Lua::String>("texture");
}

Lua::Table 
Component<Name::Text>::getTable(
    const Data& data)
{
    Lua::Table table;
    table.set("string", data.string);
    table.set("font",   data.font);
    table.set("characterSize", data.character_size);
    table.set("textAlign", (Lua::Number)(int)data.align);
    return table;
}

void 
Component<Name::Text>::fromTable(
    const Lua::Table& table, 
    void* _data)
{
    auto* data = reinterpret_cast<Data*>(_data);
    data->string = table.get<Lua::String>("string");
    data->font   = table.get<Lua::String>("font");
    data->character_size = table.get<Lua::Number>("characterSize");
    data->align = (TextAlign)(int)table.get<Lua::Number>("textAlign");
}

void 
Component<Name::Tilemap>::Map::setTile(
    int16_t x, 
    int16_t y, 
    uint32_t layer,
    const Tile& tile)
{
    if (!map.count(layer)) map.insert(std::pair(layer, std::pair(std::unordered_map<int32_t, Tile>(), LayerState::NotSolid)));
    auto& L = map.at(layer).first;
    int32_t key = (x << 16) | y;
    if (L.count(key)) L.at(key) = tile;
    else L.insert(std::pair(key, tile));
    changed = true;
}

void 
Component<Name::Tilemap>::Map::setLayerState(
    uint32_t layer, 
    LayerState state)
{
    if (!map.count(layer)) map.insert(std::pair(layer, std::pair(std::unordered_map<int32_t, Tile>(), state)));
    auto& L = map.at(layer);
    if (L.second != state) L.second = state;
    changed = true;
}

int 
Component<Name::Tilemap>::setTile(
    Lua::State L)
{
    const auto [ component_table, layer, x, y, cx, cy ] = 
        Lua::Lib::Base::extractArgs<Lua::Table, Lua::Number, Lua::Number, Lua::Number, Lua::Number, Lua::Number>(L);

    auto [ world, entity ] = Entity::extractWorldInfo(component_table);

    S2D_ASSERT(entity.is_alive(), "Entity is dead");

    const auto WORLD_ID = world.component<ComponentData<Name::Tilemap>>().raw_id();
    S2D_ASSERT(WORLD_ID == (decltype(WORLD_ID))component_table.get<Lua::Number>("type"), "Component is not Tilemap");
    auto* tilemap = entity.get_mut<ComponentData<Name::Tilemap>>();

    tilemap->tiles.setTile(x, y, layer, { { (unsigned int)cx, (unsigned int)cy } });

    Lua::CompileTime::TypeMap<Lua::Boolean>::push(L, true);

    return 1;
}

int 
Component<Name::Tilemap>::setLayerState(
    Lua::State L)
{
    const auto [ component_table, layer, layer_state ] = 
        Lua::Lib::Base::extractArgs<Lua::Table, Lua::Number, Lua::Number>(L);

    auto [ world, entity ] = Entity::extractWorldInfo(component_table);

    S2D_ASSERT(entity.is_alive(), "Entity is dead");

    const auto WORLD_ID = world.component<ComponentData<Name::Tilemap>>().raw_id();
    S2D_ASSERT(WORLD_ID == (decltype(WORLD_ID))component_table.get<Lua::Number>("type"), "Component is not Tilemap");
    auto* tilemap = entity.get_mut<ComponentData<Name::Tilemap>>();

    tilemap->tiles.setLayerState((uint32_t)layer, (LayerState)(int)layer_state);

    Lua::CompileTime::TypeMap<Lua::Boolean>::push(L, true);

    return 1;
}

Lua::Table 
Component<Name::Tilemap>::getTable(
    const Data& data)
{
    Lua::Table table;
    table.set("setTile", (Lua::Function)setTile);
    table.set("setLayerState", (Lua::Function)setLayerState);
    
    Lua::Table tilesize;
    tilesize.set("width", data.tilesize.x);
    tilesize.set("height", data.tilesize.y);
    table.set("tilesize", tilesize);

    Lua::Table spritesheet;
    spritesheet.set("textureName", data.spritesheet.texture_name);

    table.set("spriteSheet", spritesheet);

    return table;
}

void 
Component<Name::Tilemap>::fromTable(
    const Lua::Table& table, 
    void* _data)
{
    auto* data = reinterpret_cast<Data*>(_data);
    const auto& tilesize = table.get<Lua::Table>("tilesize");
    data->tilesize.x = tilesize.get<Lua::Number>("width");
    data->tilesize.y = tilesize.get<Lua::Number>("height");

    const auto& spritesheet = table.get<Lua::Table>("spritesheet");
    data->spritesheet.texture_name = spritesheet.get<Lua::String>("texture_name");
}

const char* operator*(Projection p)
{
    switch (p)
    {
    case Projection::Orthographic: return "Orthographic";
    case Projection::Perspective:  return "Perspective";
    default: return "";
    }
}

S2D::Math::Mat4f 
viewMatrix(
    flecs::entity camera)
{
    const auto camera_transform = camera.get<Transform>();
    S2D_ASSERT(camera_transform, "Camera missing transform component");

    S2D::Math::Transform view;
    view.translate(camera_transform->position * -1.f);
    return view.matrix();
}

S2D::Math::Mat4f 
projectionMatrix(
    flecs::entity camera)
{
    const auto* camera_comp = camera.get<Camera>();
    S2D_ASSERT(camera_comp, "Camera missing camera component");

    const auto aspectRatio = (float)camera_comp->size.x / (float)camera_comp->size.y;

    const auto near = 0.01;
    const auto far = 10000.0;
    const auto t = 1.f / tanf(Util::degrees(camera_comp->FOV / 2.f).asRadians());

    S2D::Math::Mat4f proj(false);
    proj[0][0] = t / aspectRatio;
    proj[1][1] = t;
    proj[2][2] = -1.f * (far + near) / (far - near);
    proj[3][2] = -2.f * (far * near) / (far - near);
    proj[2][3] = -1.f;

    return proj;
}

Lua::Table 
Component<Name::Camera>::getTable(
    const Data& data)
{
    Lua::Table table;
    table.set("FOV", data.FOV);
    table.set("projection", (Lua::Number)(int)data.projection);

    Lua::Table size;
    size.set("width",  (Lua::Number)data.size.x);
    size.set("height", (Lua::Number)data.size.y);
    table.set("size", size);

    return table;
}

void 
Component<Name::Camera>::fromTable(
    const Lua::Table& table, 
    void* _data)
{
    auto* data = reinterpret_cast<Data*>(_data);
    data->FOV = table.get<Lua::Number>("FOV");
    data->projection = (Projection)(int)table.get<Lua::Number>("projection");

    const auto& size = table.get<Lua::Table>("size");
    data->size.x = size.get<Lua::Number>("width");
    data->size.y = size.get<Lua::Number>("height");
}

Lua::Table 
Component<Name::CustomMesh>::getTable(
    const Data& data)
{
    auto table = MeshLib().asTable();
    return table;
}

void 
Component<Name::CustomMesh>::fromTable(
    const Lua::Table& table, 
    void* _data)
{
    auto* data = reinterpret_cast<Data*>(_data);
}

Lua::Table 
Component<Name::Shader>::getTable(
    const Data& data)
{
    Lua::Table table;
    table.set("name", data.name);
    return table;
}

void 
Component<Name::Shader>::fromTable(
    const Lua::Table& table, 
    void* _data)
{
    auto* data = reinterpret_cast<Data*>(_data);
    data->name = table.get<Lua::String>("name");
}

void
registerComponents(Lua::Table& table, flecs::world& world)
{
    Util::CompileTime::static_for<(int)Name::Count>([&](auto n)
    {
        constexpr std::size_t i = n;
        constexpr auto component = static_cast<Name>(i);
        using Data = ComponentData<component>;
        table.set(std::string(*component), (Lua::Number)world.component<Data>().raw_id());
    });    
}

}

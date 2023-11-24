#include <Simple2D/Engine/Mesh.hpp>
#include <Simple2D/Engine/Components.hpp>

#include <Simple2D/Log/Log.hpp>

namespace S2D::Engine
{

#define INIT_BUILD(type) \
    if (!entity.is_alive()) return;                                                         \
    S2D_ASSERT(entity.has<type>(), "Entity doesn't have the component for this mesh!");     \
    Log::Logger::instance("engine")->trace("Generating vertex information for {}", #type);  \
    mesh.vertices.clear();                                                                  \
    if (entity.has<Collider>()) {                                                           \
        const auto* collider = entity.get<Collider>();                                      \
        if (collider->collider_component == entity.world().component<type>().raw_id()) {    \
            Log::Logger::instance("engine")->trace("Building collider mesh for {}", #type); \
            auto& mesh = entity.get_mut<Collider>()->mesh;                                  \
            if (!mesh) mesh = std::make_shared<CollisionMesh>();                            \
            this->collision = mesh;                                                         \
        }                                                                                   \
    }                                                                                       \

#define MAKE_COLLISION_MODEL(type, collision) \
    if (!collision ||                                                                       \
        (collision && !collision->vertices.size()) ||                                       \
        (collision && !collision->triangles.size())) return mesh;                           \
    collision->vertices.shrink_to_fit();                                                    \
    collision->triangles.shrink_to_fit();                                                   \
    if (collision->model) delete collision->model.release();                                \
    Log::Logger::instance("engine")->trace("Generating collision model for {}", #type);     \
    collision->model = std::make_shared<CollisionMesh::Model>();                            \
    collision->model->beginModel();                                                         \
    collision->model->addSubModel(collision->vertices, collision->triangles);               \
    collision->model->endModel();    

Mesh::Mesh(flecs::entity _entity) :
    entity(_entity)
{   }

Mesh::~Mesh()
{   }                                                       

static void 
addBox(
    std::shared_ptr<CollisionMesh>& collision,
    sf::Vector2u size,
    sf::Vector3f offset,
    float scale)
{
    const std::vector<sf::Vector3f> offsets = {
        { -0.5, -0.5,  0.5 },
        {  0.5, -0.5,  0.5 },
        { -0.5,  0.5,  0.5 },
        {  0.5,  0.5,  0.5 },
        { -0.5, -0.5, -0.5 },
        {  0.5, -0.5, -0.5 },
        { -0.5,  0.5, -0.5 },
        {  0.5,  0.5, -0.5 }
    };

    const std::vector<fcl::Triangle> indices = {
        //Top
        { 2, 6, 7 },
        { 2, 3, 7 },

        //Bottom
        { 0, 4, 5 },
        { 0, 1, 5 },

        //Left
        { 0, 2, 6 },
        { 0, 4, 6 },

        //Right
        { 1, 3, 7 },
        { 1, 5, 7 },

        //Front
        { 0, 2, 3 },
        { 0, 1, 3 },

        //Back
        { 4, 6, 7 },
        { 4, 5, 7 }
    };

    std::size_t init_offset = collision->vertices.size();
    for (const auto& vertex : offsets)
    {
        collision->vertices.push_back({ 
            (vertex.x * size.x + offset.x) * scale, 
            (vertex.y * size.y + offset.y) * scale, 
            vertex.z + offset.z 
        });
    }

    for (const auto& triangle : indices)
        collision->triangles.push_back({ init_offset + triangle[0], init_offset + triangle[1], init_offset + triangle[2] });
}

template<>
Mesh MeshBuilder<Sprite>::get(flecs::entity e)
{
    S2D_ASSERT(e.is_alive(), "Trying to build sprite mesh for dead entity!");
    S2D_ASSERT(e.has<Sprite>(), "Trying to build sprite mesh for entity that doesn't have sprite!");

    Mesh mesh(e);    
    
    if (e.has<Collider>()) {
        const auto* collider = e.get<Collider>();
        if (collider->collider_component == e.world().component<Sprite>().raw_id()) {
            auto& collision_mesh = e.get_mut<Collider>()->mesh;
            if (!collision_mesh) collision_mesh = std::make_shared<CollisionMesh>();
            mesh.collision = collision_mesh;
        }
    } 
    
    if (!mesh.raw_mesh) mesh.raw_mesh = std::make_shared<RawMesh>();

    const auto scale = (e.has<Transform>() ? e.get<Transform>()->scale : 1.f);
    const auto* sprite = e.get<Sprite>();

    // Forming a quad from two triangles
    const sf::Vector2f offsets[] = {
        { -0.5f,  0.5f },
        {  0.5f, -0.5f },
        { -0.5f, -0.5f },

        {  0.5f, -0.5f },
        { -0.5f,  0.5f },
        {  0.5f,  0.5f }
    };

    const sf::Vector2f tex_coords[] = {
        { 0, 1 },
        { 1, 0 },
        { 0, 0 },

        { 1, 0 },
        { 0, 1 },
        { 1, 1 }
    };

    mesh.raw_mesh->vertices.clear();

    mesh.raw_mesh->vertices.resize(6);
    mesh.raw_mesh->vertices.setPrimitiveType(sf::PrimitiveType::Triangles);

    for (uint32_t i = 0; i < 6; i++)
    {
        auto& vertex = mesh.raw_mesh->vertices[i];
        vertex.position  = sf::Vector2f(offsets[i].x * sprite->size.x, offsets[i].y * sprite->size.y);
        vertex.texCoords = tex_coords[i];
        vertex.color = sf::Color::White;
    }

    MAKE_COLLISION_MODEL(Sprite);

    return mesh;
}


/* Sprite */
/*
template<>
void TypeMesh<Sprite>::build()
{
    INIT_BUILD(Sprite);

    const auto scale = (entity.has<Transform>() ? entity.get<Transform>()->scale : 1.f);
    const auto* sprite = entity.get<Sprite>();

    // Forming a quad from two triangles
    const sf::Vector2f offsets[] = {
        { -0.5f,  0.5f },
        {  0.5f, -0.5f },
        { -0.5f, -0.5f },

        {  0.5f, -0.5f },
        { -0.5f,  0.5f },
        {  0.5f,  0.5f }
    };

    const sf::Vector2f tex_coords[] = {
        { 0, 1 },
        { 1, 0 },
        { 0, 0 },

        { 1, 0 },
        { 0, 1 },
        { 1, 1 }
    };

    if (collision) 
    {
        collision->vertices.clear(); 
        collision->triangles.clear();
        addBox(collision, sprite->size, { 0, 0, 0 }, scale);
    }
    
    mesh.vertices.clear();

    mesh.vertices.resize(6);
    mesh.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);

    for (uint32_t i = 0; i < 6; i++)
    {
        auto& vertex = mesh.vertices[i];
        vertex.position  = sf::Vector2f(offsets[i].x * sprite->size.x, offsets[i].y * sprite->size.y);
        vertex.texCoords = tex_coords[i];
        vertex.color = sf::Color::White;
    }

    MAKE_COLLISION_MODEL(Sprite);
}

template<>
TypeMesh<Sprite>::TypeMesh(flecs::entity e) :
    Mesh(e)
{
    build();
}*/

/* Tilemap */
/*
template<>
void
TypeMesh<Tilemap>::build()
{
    INIT_BUILD(Tilemap);

    const auto scale = (entity.has<Transform>() ? entity.get<Transform>()->scale : 1.f);
    const auto* tilemap = entity.get<Tilemap>();
    const auto& map = tilemap->tiles.map;

    if (collision) { collision->vertices.clear(); collision->triangles.clear(); }
    
    // Initialize the vertex array and render states
    const auto total_tiles = [&]()
    {
        uint32_t count = 0;
        for (const auto& p : map)
            for (const auto& t : p.second.first)
                count++;
        return count;
    }();

    uint32_t iterator = 0;
    mesh.vertices.resize(total_tiles * 6);
    mesh.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);


    for (const auto& p : map)
    {
        for (const auto& t : p.second.first)
        {
            // Extract the coordinate info from the key
            const int32_t mask = 0xFFFF0000;
            int16_t x = ((t.first & mask) >> 16);
            int16_t y = (t.first & (~mask));
            const auto position = sf::Vector2f(
                x * tilemap->tilesize.x,
                y * tilemap->tilesize.y
            );

            // Construct the texture rectangle from the tile information
            sf::IntRect tex_rect;
            tex_rect.width  = tilemap->tilesize.x;
            tex_rect.height = tilemap->tilesize.y;
            tex_rect.left = t.second.texture_coords.x * tilemap->tilesize.x;
            tex_rect.top  = t.second.texture_coords.y * tilemap->tilesize.y;

            // Forming a quad from two triangles
            const sf::Vector2f offsets[] = {
                { -0.5f,  0.5f },
                {  0.5f, -0.5f },
                { -0.5f, -0.5f },

                {  0.5f, -0.5f },
                { -0.5f,  0.5f },
                {  0.5f,  0.5f }
            };

            const sf::Vector2i tex_coords[] = {
                { 0, 1 },
                { 1, 0 },
                { 0, 0 },

                { 1, 0 },
                { 0, 1 },
                { 1, 1 }
            };

            // Possibly... Now we need to push cubes to the collider mesh, not quads
            if (collision && p.second.second == Component<Name::Tilemap>::LayerState::Solid)
            {
                // Push quad at this point
                addBox(collision, (sf::Vector2u)tilemap->tilesize, { position.x, position.y, 0 }, scale);
            }
            
            // Construct the vertices of the quad
            for (uint8_t i = 0; i < 6; i++)
            {
                auto& vertex = mesh.vertices[iterator++];
                vertex.position.x = offsets[i].x * tilemap->tilesize.x + position.x;
                vertex.position.y = offsets[i].y * tilemap->tilesize.y + position.y;
                vertex.color = sf::Color::White;  
                vertex.texCoords.x = tex_rect.left + tex_rect.width  * tex_coords[i].x;
                vertex.texCoords.y = tex_rect.top + tex_rect.height * tex_coords[i].y;  
            }
        }
    }

    MAKE_COLLISION_MODEL(Tilemap);
}

template<>
TypeMesh<Tilemap>::TypeMesh(flecs::entity entity) :
    Mesh(entity)
{
    build();
}*/

}

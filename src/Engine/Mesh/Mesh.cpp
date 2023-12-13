#include <Simple2D/Engine/Mesh.hpp>
#include <Simple2D/Engine/Components.hpp>

#include <Simple2D/Log/Log.hpp>

namespace S2D::Engine
{

const char* operator*(Primitive p)
{
    switch (p)
    {
    case Primitive::Lines:     return "Lines";
    case Primitive::Points:    return "Points";
    case Primitive::Triangles: return "Triangles";
    default: return "";
    }
}

#define REQUIRE(expr) if (!(expr)) return

static void constructSprite(Graphics::VertexArray& vertices)
{
    using namespace Graphics;

    // Forming a quad from two triangles
    const Math::Vec3f offsets[] = {
        { -0.5f,  0.5f, 0.f },
        {  0.5f, -0.5f, 0.f },
        { -0.5f, -0.5f, 0.f },
        {  0.5f,  0.5f, 0.f }
    };

    const Math::Vec2f tex_coords[] = {
        { 0, 1 },
        { 1, 0 },
        { 0, 0 },
        { 1, 1 }
    };

    vertices.setDrawType(VertexArray::DrawType::Triangles);

    std::vector<Vertex> verts(4);
    const std::vector<uint32_t> indices = {0, 1, 2, 0, 3, 1};

    uint32_t i = 0;
    for (auto& vertex : verts)
    {
        vertex.position  = offsets[i];
        vertex.texCoords = tex_coords[i++];
        vertex.color = Color(255, 255, 255, 255);
    }
    vertices.upload(verts);
    vertices.uploadIndices(indices);
}

std::shared_ptr<RawMesh> RawMesh::getQuadMesh()
{
    static std::shared_ptr<RawMesh> quadMesh;

    if (!quadMesh)
    {
        Log::Logger::instance("engine")->info("building quad mesh");
        quadMesh = std::make_shared<RawMesh>();
        constructSprite(quadMesh->vertices);
    }

    return quadMesh;
}

template<>
void MeshBuilder<Sprite>::checkAndBuild(flecs::entity e)
{
    // Would much rather this be stored in the scene somehow
    //static std::shared_ptr<RawMesh> data;

    // Make sure the entity has a sprite
    REQUIRE(e.has<Sprite>());
    auto* sprite = e.get_mut<Sprite>();

    // If the sprite's mesh hasn't been generated, go ahead and generate it
    if (!sprite->mesh) sprite->mesh = RawMesh::getQuadMesh();
}

template<>
void MeshBuilder<Tilemap>::checkAndBuild(flecs::entity e)
{
    REQUIRE(e.has<Tilemap>());
    auto* tilemap = e.get_mut<Tilemap>();

    const auto tilemap_changed = tilemap->tiles.changed;
    tilemap->tiles.changed = false;

    bool build_mesh = false;
    // If the tilemap has been changed, destroy the mesh and start over
    if (tilemap_changed)
    {
        auto* ptr = tilemap->mesh.release();
        delete ptr;

        build_mesh = true;
    }

    if (!tilemap->mesh)
    {
        // Construct the mesh
        tilemap->mesh = std::make_unique<RawMesh>();
        build_mesh = true;
    }

    if (!build_mesh) return;

    const auto& map = tilemap->tiles.map;

    // Now we build the mesh
    Log::Logger::instance("engine")->info("building tilemap");

    // Initialize the vertex array and render states
    const auto total_tiles = [&]()
    {
        uint32_t count = 0;
        for (const auto& p : map)
            for (const auto& t : p.second.first)
                count++;
        return count;
    }();

    using namespace Graphics;
    uint32_t iterator = 0, index_iterator = 0;
    std::vector<Vertex> vertices(4 * total_tiles);
    std::vector<uint32_t> indices(6 * total_tiles);

    tilemap->mesh->vertices.setDrawType(VertexArray::DrawType::Triangles);

    const auto scale = (e.has<Transform>()?e.get<Transform>()->scale:1.f);

    for (const auto& p : map)
    {
        for (const auto& t : p.second.first)
        {
            // Extract the coordinate info from the key
            const int32_t mask = 0xFFFF0000;
            int16_t x = ((t.first & mask) >> 16);
            int16_t y = (t.first & (~mask));
            const Math::Vec2f position = {
                x * tilemap->tilesize.x,
                y * tilemap->tilesize.y
            };

            // Construct the texture rectangle from the tile information
            Math::Vec2f pos, size;
            size.x  = tilemap->tilesize.x;
            size.y = tilemap->tilesize.y;
            pos.x = t.second.texture_coords.x * tilemap->tilesize.x;
            pos.y = t.second.texture_coords.y * tilemap->tilesize.y;

            // Forming a quad from two triangles
            const Math::Vec2f offsets[] = {
                { -0.5f,  0.5f },
                {  0.5f, -0.5f },
                { -0.5f, -0.5f },
                {  0.5f,  0.5f }
            };

            const Math::Vec2f tex_coords[] = {
                { 0, 1 },
                { 1, 0 },
                { 0, 0 },
                { 1, 1 }
            };

            const std::vector<uint32_t> subindices = {
                0, 1, 2, 2, 3, 0
            };
            
            // Construct the vertices of the quad
            for (uint8_t i = 0; i < 4; i++)
            {
                auto& vertex = vertices[iterator++];
                vertex.position.x = offsets[i].x * tilemap->tilesize.x + position.x;
                vertex.position.y = offsets[i].y * tilemap->tilesize.y + position.y;
                vertex.color = Color(255, 255, 255, 255);  
                vertex.texCoords.x = pos.x + size.x * tex_coords[i].x;
                vertex.texCoords.y = pos.y + size.y * tex_coords[i].y;  
            }

            const auto start_size = index_iterator;
            for (uint32_t i = 0; i < 6; i++)
                indices[index_iterator++] = subindices[i] + start_size;
        }
    }

    tilemap->mesh->vertices.upload(vertices);
    tilemap->mesh->vertices.uploadIndices(indices);
}

}

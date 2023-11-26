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

#define MAKE_COLLISION_MODEL(type, collision) \
    if (!collision ||                                                                       \
        (collision && !collision->vertices.size()) ||                                       \
        (collision && !collision->triangles.size())) return;                                \
    collision->vertices.shrink_to_fit();                                                    \
    collision->triangles.shrink_to_fit();                                                   \
    Log::Logger::instance("engine")->trace("Generating collision model for {}", #type);     \
    collision->model = std::make_shared<CollisionMesh::Model>();                            \
    collision->model->beginModel();                                                         \
    collision->model->addSubModel(collision->vertices, collision->triangles);               \
    collision->model->endModel();    

    static void 
    addBox(
        std::unique_ptr<CollisionMesh>& collision,
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

    static void constructSprite(sf::VertexArray& vertices)
    {
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

        vertices.clear();

        vertices.resize(6);
        vertices.setPrimitiveType(sf::PrimitiveType::Triangles);

        for (uint32_t i = 0; i < 6; i++)
        {
            auto& vertex = vertices[i];
            vertex.position  = sf::Vector2f(offsets[i].x, offsets[i].y);
            vertex.texCoords = tex_coords[i];
            vertex.color = sf::Color::White;
        }
    }

    //static void constructTilemap()

    template<>
    void MeshBuilder<Sprite>::checkAndBuild(flecs::entity e)
    {
        // Would much rather this be stored in the scene somehow
        static std::shared_ptr<RawMesh> data;

        // Make sure the entity has a sprite
        REQUIRE(e.has<Sprite>());
        auto* sprite = e.get_mut<Sprite>();
        auto* collider = (e.has<Collider>()?e.get_mut<Collider>():nullptr);

        // If the sprite's mesh hasn't been generated, go ahead and generate it
        if (!sprite->mesh)
        {
            if (!data)
            {
                Log::Logger::instance("engine")->info("building sprite");
                data = std::make_shared<RawMesh>();
                constructSprite(data->vertices);
            }
            sprite->mesh = data;
        }

        // Set flag to re-build the collider mesh when transform is changed (?)
        // If the collider's mesh hasn't been initialized go ahead and construct it
        if (collider && !collider->mesh)
        {
            Log::Logger::instance("engine")->info("building sprite collider");
            collider->mesh = std::make_unique<CollisionMesh>();

            // Grab the scale for the collision model
            const auto scale = (e.has<Transform>()?e.get<Transform>()->scale:1.f);

            // Generate the collision mesh
            collider->mesh->vertices.clear(); 
            collider->mesh->triangles.clear();
            addBox(collider->mesh, sprite->size, { 0, 0, 0 }, scale);
            
            MAKE_COLLISION_MODEL(Sprite, collider->mesh);
        }
    }

    template<>
    void MeshBuilder<Tilemap>::checkAndBuild(flecs::entity e)
    {
        REQUIRE(e.has<Tilemap>());
        auto* tilemap = e.get_mut<Tilemap>();
        auto* collider = (e.has<Collider>()?e.get_mut<Collider>():nullptr);

        const auto tilemap_changed = tilemap->tiles.changed;
        tilemap->tiles.changed = false;

        bool build_mesh = false, build_collider = false;
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

        // If the tilemap has been changed, destroy the collision mesh and start over
        if (tilemap_changed)
        {
            auto* ptr = collider->mesh.release();
            delete ptr;

            build_collider = true;
        }

        if (!collider->mesh)
        {
            // Construct the collision mesh
            collider->mesh = std::make_unique<CollisionMesh>();

            build_collider = true;
        }

        if (!build_collider && !build_mesh) return;

        if (build_collider) Log::Logger::instance("engine")->info("building tilemap collider");

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

        uint32_t iterator = 0;
        tilemap->mesh->vertices.resize(total_tiles * 6);
        tilemap->mesh->vertices.setPrimitiveType(sf::PrimitiveType::Triangles);

        const auto scale = (e.has<Transform>()?e.get<Transform>()->scale:1.f);

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
                if (collider && p.second.second == Component<Name::Tilemap>::LayerState::Solid)
                {
                    // Push quad at this point
                    addBox(collider->mesh, (sf::Vector2u)tilemap->tilesize, { position.x, position.y, 0 }, scale);
                }
                
                // Construct the vertices of the quad
                for (uint8_t i = 0; i < 6; i++)
                {
                    auto& vertex = tilemap->mesh->vertices[iterator++];
                    vertex.position.x = offsets[i].x * tilemap->tilesize.x + position.x;
                    vertex.position.y = offsets[i].y * tilemap->tilesize.y + position.y;
                    vertex.color = sf::Color::White;  
                    vertex.texCoords.x = tex_rect.left + tex_rect.width  * tex_coords[i].x;
                    vertex.texCoords.y = tex_rect.top + tex_rect.height * tex_coords[i].y;  
                }
            }
        }

        REQUIRE(e.has<Collider>());
        MAKE_COLLISION_MODEL(Tilemap, collider->mesh);
    }
}
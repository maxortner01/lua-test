#include <Simple2D/Engine/Core.hpp>

namespace S2D::Engine
{

void RenderComponent(
    Scene* scene,
    flecs::entity e,
    const Transform& transform,
    Tilemap* tilemap,
    sf::RenderTarget& target)
{
    // Construct or re-construct the mesh for the tilemap
    if (!tilemap->mesh)
    {
        tilemap->mesh = std::unique_ptr<Mesh>(
            new TypeMesh<Tilemap>(e)
        );
        tilemap->tiles.changed = false;
    }
    else if (tilemap->tiles.changed)
    {
        tilemap->mesh->build();
        tilemap->tiles.changed = false;
    }

    sf::RenderStates states;

    // If the tilemap has a texture (it should always) set the state
    if (tilemap->spritesheet.texture_name.size())
    {
        auto texture = scene->resources.getResource<sf::Texture>(tilemap->spritesheet.texture_name);
        S2D_ASSERT(texture, "Error getting texture");
        states.texture = texture.value();
    }
    
    // This work will be done in the shader...
    auto vertices = tilemap->mesh->mesh.vertices;
    for (uint32_t i = 0; i < vertices.getVertexCount(); i++)
    {
        auto& vertex = vertices[i];
        vertex.position *= transform.scale;
        vertex.position += sf::Vector2f(transform.position.x, transform.position.y);
    }
    
    target.draw(vertices, states);

    // Debug: Render the collision mesh
    const auto* collider = e.get<Collider>();
    if (collider)
    {
        sf::VertexArray collider_points(sf::PrimitiveType::Triangles, collider->mesh->triangles.size() * 3);

        uint32_t iterator = 0;
        for (uint32_t i = 0; i < collider->mesh->triangles.size(); i++)
        {
            for (uint32_t t = 0; t < 3; t++)
            {
                const auto& vertex = collider->mesh->vertices[collider->mesh->triangles[i][t]];
                auto& collider_point = collider_points[iterator++];
                collider_point.position = sf::Vector2f(vertex[0], vertex[1]);
                collider_point.color = sf::Color::Red;

                collider_point.position += sf::Vector2f(transform.position.x / transform.scale, transform.position.y / transform.scale);
            }
        }

        target.draw(collider_points);
    }
}

void RenderComponent(
    Scene* scene,
    flecs::entity e,
    const Transform& transform,
    Text* text,
    sf::RenderTarget& target)
{
    auto* font = scene->resources.getResource<sf::Font>(text->font).value();
    S2D_ASSERT(font, "Font is null");

    int max_height = std::numeric_limits<int>::min();
    for (const auto& c : text->string)
    {
        const auto height = font->getGlyph('c', text->character_size, false).bounds.height;
        if (height > max_height) max_height = height;
    }

    sf::Text t(*font, text->string);
    t.setPosition(sf::Vector2f(
        (int)transform.position.x,
        (int)transform.position.y
    ));
    t.setOrigin(sf::Vector2f(
        (int)t.getLocalBounds().left /*+ (int)(t.getLocalBounds().width / 2.f)*/,
        (int)t.getLocalBounds().top + (int)(max_height / 2.f)
    ));
    t.setCharacterSize((unsigned int)text->character_size);
    t.setFillColor(sf::Color::White);

    target.draw(t);
}

void RenderComponent(
    Scene* scene,
    flecs::entity e,
    const Transform& transform,
    Sprite* sprite,
    sf::RenderTarget& target)
{
    sf::RenderStates states;
    if (sprite->texture.size())
        states.texture = scene->resources.getResource<sf::Texture>(sprite->texture).value();

    if (!sprite->mesh)
        sprite->mesh = std::unique_ptr<Mesh>(new TypeMesh<Sprite>(e));

    // This work will be done in the shader...
    auto vertices = sprite->mesh->mesh.vertices;
    for (uint32_t i = 0; i < vertices.getVertexCount(); i++)
    {
        auto& vertex = vertices[i];
        vertex.position *= transform.scale;
        vertex.position += sf::Vector2f(transform.position.x, transform.position.y);
    }

    target.draw(vertices, states);

    // [Debug] Draw the collider mesh
    const auto* collider = e.get<Collider>();
    if (collider)
    {
        sf::VertexArray collider_points(sf::PrimitiveType::Triangles, collider->mesh->triangles.size() * 3);

        uint32_t iterator = 0;
        for (uint32_t i = 0; i < collider->mesh->triangles.size(); i++)
        {
            for (uint32_t t = 0; t < 3; t++)
            {
                const auto& vertex = collider->mesh->vertices[collider->mesh->triangles[i][t]];
                auto& collider_point = collider_points[iterator++];
                collider_point.position = sf::Vector2f(vertex[0], vertex[1]);
                collider_point.color = sf::Color::Red;

                collider_point.position += sf::Vector2f(transform.position.x / transform.scale, transform.position.y / transform.scale);
            }
        }

        /*
        for (uint32_t i = 0; i < collider_points.getVertexCount(); i++)
        {
            auto& vertex = collider_points[i];
            const auto& collider_point = collider->mesh->vertices[i];
            vertex.position = sf::Vector2f(collider_point[0], collider_point[1]);
            vertex.color = sf::Color::Red;

            //vertex.position *= transform.scale;
            vertex.position += sf::Vector2f(transform.position.x / transform.scale, transform.position.y / transform.scale);
        }*/

        target.draw(collider_points);
    }
}

void Core::render(Scene* scene)
{   
    scene->transforms.each(
        [&](flecs::entity e, const ComponentData<Name::Transform>& transform)
        {
            /* Render Sprite */
            if (e.has<Sprite>()) RenderComponent(scene, e, transform, e.get_mut<Sprite>(), window);

            /* Render Text */
            if (e.has<Text>()) RenderComponent(scene, e, transform, e.get_mut<Text>(), window);

            /* Render Tilemap */
            if (e.has<Tilemap>()) RenderComponent(scene, e, transform, e.get_mut<Tilemap>(), window);
        }
    );
}

}
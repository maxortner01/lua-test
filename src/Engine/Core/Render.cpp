#include <Simple2D/Engine/Core.hpp>

#include <Simple2D/Engine/LuaLib/Surface.hpp>

#include <Simple2D/Log/Log.hpp>

namespace S2D::Engine
{

void RenderComponent(
    Scene* scene,
    flecs::entity e,
    flecs::entity camera,
    const Transform& transform,
    Tilemap* tilemap,
    sf::RenderTarget& target)
{
    // Construct or re-construct the mesh for the tilemap
    MeshBuilder<Tilemap>::checkAndBuild(e);

    sf::Transform model, view;
    model.translate(sf::Vector2f(transform.position.x, transform.position.y));
    model.rotate(sf::degrees(transform.rotation));
    model.scale(sf::Vector2f(transform.scale, transform.scale));

    view.translate(-1.f * sf::Vector2f(
        camera.has<Transform>() ? camera.get<Transform>()->position.x : 0.f,
        camera.has<Transform>() ? camera.get<Transform>()->position.y : 0.f
    ));

    sf::RenderStates states;

    // If the tilemap has a texture (it should always) set the state
    if (tilemap->spritesheet.texture_name.size())
    {
        auto texture = scene->resources.getResource<sf::Texture>(tilemap->spritesheet.texture_name);
        S2D_ASSERT(texture, "Error getting texture");
        states.texture = texture.value();
    }
    
    // This work will be done in the shader...
    auto vertices = tilemap->mesh->vertices;
    for (uint32_t i = 0; i < vertices.getVertexCount(); i++)
    {
        auto& vertex = vertices[i];
        
        vertex.position = model.transformPoint(vertex.position);
        vertex.position = view.transformPoint(vertex.position);
        vertex.position += (sf::Vector2f)target.getSize() / 2.f;
    }
    
    target.draw(vertices, states);

    return;

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
                collider_point.color.a = 50;

                collider_point.position += sf::Vector2f(transform.position.x, transform.position.y);
            }
        }

        target.draw(collider_points);
    }
}

void RenderComponent(
    Scene* scene,
    flecs::entity e,
    flecs::entity camera,
    const Transform& transform,
    Text* text,
    sf::RenderTarget& target)
{
    auto* font = scene->resources.getResource<sf::Font>(text->font).value();
    S2D_ASSERT(font, "Font is null");

    auto camera_pos = sf::Vector2f(
        camera.has<Transform>() ? camera.get<Transform>()->position.x : 0.f,
        camera.has<Transform>() ? camera.get<Transform>()->position.y : 0.f
    );

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
    ) - camera_pos + (sf::Vector2f)target.getSize() / 2.f);
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
    flecs::entity camera,
    const Transform& transform,
    Sprite* sprite,
    sf::RenderTarget& target)
{
    sf::RenderStates states;
    if (sprite->texture.size())
        states.texture = scene->resources.getResource<sf::Texture>(sprite->texture).value();

    MeshBuilder<Sprite>::checkAndBuild(e);

    auto camera_pos = sf::Vector2f(
        camera.has<Transform>() ? camera.get<Transform>()->position.x : 0.f,
        camera.has<Transform>() ? camera.get<Transform>()->position.y : 0.f
    );

    S2D_ASSERT(sprite->mesh, "Error generating mesh");

    sf::Transform model, view;
    model.translate(sf::Vector2f(transform.position.x, transform.position.y));
    model.rotate(sf::degrees(transform.rotation));
    model.scale(sf::Vector2f(transform.scale, transform.scale));

    view.translate(-1.f * camera_pos);

    // This work will be done in the shader...
    auto vertices = sprite->mesh->vertices;
    for (uint32_t i = 0; i < vertices.getVertexCount(); i++)
    {
        auto& vertex = vertices[i];
        vertex.position.x *= sprite->size.x;
        vertex.position.y *= sprite->size.y;

        if (states.texture)
        {
            vertex.texCoords.x *= states.texture->getSize().x;
            vertex.texCoords.y *= states.texture->getSize().x;
        }

        vertex.position = model.transformPoint(vertex.position);
        vertex.position = view.transformPoint(vertex.position);
        vertex.position += (sf::Vector2f)target.getSize() / 2.f;
    }

    target.draw(vertices, states);

    return;

    // [Debug] Draw the collider mesh
    const auto* collider = e.get<Collider>();
    S2D_ASSERT(collider && collider->mesh, "Error generating mesh");
    
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
                collider_point.color.a = 50;

                collider_point.position += sf::Vector2f(transform.position.x, transform.position.y);
            }
        }

        target.draw(collider_points);
    }
}

void RenderComponent(
    Scene* scene,
    flecs::entity e,
    flecs::entity camera,
    const Transform& transform,
    CustomMesh* mesh,
    sf::RenderTarget& target)
{
    if (!mesh->mesh) return;

    sf::RenderStates states;

    const auto* shader_component = e.get<ShaderComp>();
    if (shader_component)
    {
        states.shader = [&]()
        {
            auto res = scene->resources.getResource<sf::Shader>(shader_component->name);
            S2D_ASSERT(res, "Error loading shader");

            auto* shader = res.value();
            if (camera.has<Transform>()) shader->setUniform("camera_pos", camera.get<Transform>()->position);
            shader->setUniform("target_size", (sf::Vector2f)target.getSize());

            // Load the resources
            // How can we load them into an array in SFML?
            // Don't think we can
            for (const auto& p : shader_component->textures)
            {
                switch (p.first)
                {
                case TexSource::Renderpass:
                {
                    S2D_ASSERT(scene->renderpass->targets.count(p.second), "Renderpass doesn't contain requested resource");
                    const auto& pass = scene->renderpass->targets.at(p.second);
                    pass->display();
                    shader->setUniform("size", (sf::Vector2f)pass->getSize());
                    shader->setUniform("texture", pass->getTexture());

                    return shader;
                }
                case TexSource::Resources:
                {
                    const auto resource = scene->resources.getResource<sf::Texture>(p.second);
                    S2D_ASSERT(resource, "Failed to load resource!");
                    shader->setUniform("size", (sf::Vector2f)resource.value()->getSize());
                    shader->setUniform("texture", *resource.value());
                    return shader;
                }
                default: break;
                }
            }

            return shader;
        }();


    }

    auto& logger = Log::Logger::instance("engine");

    sf::Transform model, view;
    model.translate(sf::Vector2f(transform.position.x, transform.position.y));
    model.scale(sf::Vector2f(transform.scale, transform.scale));
    model.rotate(sf::degrees(transform.rotation));

    view.translate(-1.f * sf::Vector2f(
        camera.has<Transform>() ? camera.get<Transform>()->position.x : 0.f,
        camera.has<Transform>() ? camera.get<Transform>()->position.y : 0.f
    ));

    auto vertices = mesh->mesh->vertices;
    switch (mesh->mesh->primitive)
    {
    case Primitive::Lines:     vertices.setPrimitiveType(sf::PrimitiveType::Lines);     break;
    case Primitive::Points:    vertices.setPrimitiveType(sf::PrimitiveType::Points);    break;
    case Primitive::Triangles: vertices.setPrimitiveType(sf::PrimitiveType::Triangles); break;
    case Primitive::Count: return;
    }

    for (uint32_t i = 0; i < vertices.getVertexCount(); i++)
    {
        auto& vertex = vertices[i];


        vertex.position = model.transformPoint(vertex.position);
        vertex.position = view.transformPoint(vertex.position);
        vertex.position += (sf::Vector2f)target.getSize() / 2.f;
    }

    target.draw(vertices, states);
}

static void 
render_entity(
    Scene* scene, 
    sf::RenderTarget& target, 
    const Transform& transform,
    flecs::entity camera, 
    flecs::entity e)
{
    /* Render Sprite */
    if (e.has<Sprite>()) RenderComponent(scene, e, camera, transform, e.get_mut<Sprite>(), target);

    /* Render Text */
    if (e.has<Text>()) RenderComponent(scene, e, camera, transform, e.get_mut<Text>(), target);

    /* Render Tilemap */
    if (e.has<Tilemap>()) RenderComponent(scene, e, camera, transform, e.get_mut<Tilemap>(), target);

    /* Render Custom Mesh */
    if (e.has<CustomMesh>()) RenderComponent(scene, e, camera, transform, e.get_mut<CustomMesh>(), target);
}

static void 
render_entities(Scene* scene, sf::RenderTarget& target, flecs::entity camera)
{
    scene->transforms.each(
        [&](flecs::entity e, const Transform& transform)
        {
            render_entity(scene, target, transform, camera, e);
        }
    );
}

void Core::render(Scene* scene)
{   
    auto& log = Log::Logger::instance("engine");

    sf::RenderTexture* current_target = nullptr;
    const auto& targets  = scene->renderpass->targets;
    const auto& commands = scene->renderpass->commands;

    const auto camera = scene->world.filter<const Camera>().first();

    #define GET_PARAMS(cmd) auto* params = (CommandParameters<cmd>*)command.second.get();

    for (const auto& command : commands)
    {
        switch (command.first)
        {
        case Command::BindSurface:
        {
            GET_PARAMS(Command::BindSurface);
            if (targets.count(params->name)) current_target = targets.at(params->name).get();
            else log->error("Requesting target that doesn't exist");
            break;
        }
        case Command::Clear:
        {
            if (!current_target)
            {
                log->error("Attempting to clear null surface");
                break;
            }

            GET_PARAMS(Command::Clear);
            current_target->clear(params->color);

            break;
        }
        case Command::RenderEntities:
        {
            if (!current_target)
            {
                log->error("Attempting to render entities to null surface");
                break;
            }

            GET_PARAMS(Command::RenderEntities);

            auto camera_to_use = (params->camera_name.size() ? scene->world.lookup(params->camera_name.c_str()) : camera);
            if (!camera_to_use)
            {
                log->error("Error using camera while rendering entities");
                break;
            }

            render_entities(scene, *current_target, camera_to_use);

            break;
        }
        case Command::RenderEntity:
        {
            GET_PARAMS(Command::RenderEntity);

            if (!current_target)
            {
                log->error("Attempting to render entity \"{}\" to null surface", params->entity_name);
                break;
            }

            auto entity = scene->world.lookup(params->entity_name.c_str());
            if (!entity) 
            {
                log->error("Entity \"{}\" doesn't exist", params->entity_name);
                break;
            }

            const auto* transform = entity.get<Transform>();
            if (!transform)
            {
                log->error("Entity \"{}\" doesn't have a transform", params->entity_name);
                break;
            }

            auto camera_to_use = (params->camera_name.size() ? scene->world.lookup(params->camera_name.c_str()) : camera);
            if (!camera_to_use)
            {
                log->error("Error using camera while rendering entity \"{}\"", params->entity_name);
                break;
            }

            render_entity(scene, *current_target, *transform, camera_to_use, entity);

            break;
        }
        case Command::BlitSurface:
        {
            if (!current_target)
            {
                log->error("Attempting to blit null surface to window");
                break;
            }

            GET_PARAMS(Command::BlitSurface);
            sf::RectangleShape blit;
            blit.setPosition(params->position);
            blit.setSize(params->size);

            current_target->display();
            const auto& tex = current_target->getTexture();
            blit.setTexture(&tex);
            window.draw(blit);

            break;
        }
        case Command::RenderUI:
        {
            if (!current_target)
            {
                log->error("Attempting to render UI on null surface");
                break;
            }
            
            GET_PARAMS(Command::RenderUI);
            auto& runtime = params->runtime;

            // We pass a surface to the runtime which contains the current scene
            // this way the function can access the scene and its resources
            // We also pass a pointer to the current surface
            auto surface = Surface().asTable();
            surface.set<uint64_t>("scene", (uint64_t)scene);
            surface.set<uint64_t>("surface", (uint64_t)current_target);
            
            runtime->runFunction<>("RenderUI", surface);

            break;
        }
        default: break;
        }
    }
}

}

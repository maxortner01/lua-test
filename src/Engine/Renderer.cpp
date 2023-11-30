#include <Simple2D/Engine/Renderer.hpp>
#include <Simple2D/Engine/Mesh.hpp>
#include <Simple2D/Engine/Core.hpp>

#include <Simple2D/Log/Log.hpp>

namespace S2D::Engine
{
    template<>
    DefaultShader<Sprite>::DefaultShader()
    {
        S2D_ASSERT(sf::Shader::isAvailable(), "Shaders are not supported on this machine");

        const std::string vertex = \
            "uniform mat3 model;" \
            "uniform mat3 view;" \
            "uniform float camera_z;" \
            "uniform float aspectRatio;" \
            "uniform vec2 sprite_size;" \
            "void main() " \
            "{ " \
            "  vec3 prepos = gl_Vertex.xyz; " \
            "  prepos.x *= sprite_size.x; " \
            "  prepos.y *= sprite_size.y; " \
            "  vec4 pos = vec4((prepos * model).xy, gl_Vertex.z, gl_Vertex.w);" \
            "  pos.w *= camera_z;" \
            "  pos.x += model[2][0] + view[2][0];" \
            "  pos.y += model[2][1] + view[2][1];" \
            "  pos.x /= aspectRatio;" \
            "  gl_Position = pos; " \
            "  gl_TexCoord[0] = gl_MultiTexCoord0; " \
            "  gl_FrontColor = gl_Color; " \
            "}";

        const std::string fragment = \
            "uniform sampler2D texture;" \
            "void main() " \
            "{ " \
            "  vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);" \
            "  gl_FragColor = gl_Color * pixel;" \
            "} ";
        
        S2D_ASSERT(shader.loadFromMemory(vertex, fragment), "Sprite shader failed to load");
    }

    template<>
    DefaultShader<Tilemap>::DefaultShader()
    {

    }

    template<>
    DefaultShader<CustomMesh>::DefaultShader()
    {
        const std::string vertex = \
            "uniform mat3 model;" \
            "uniform mat3 view;" \
            "uniform float camera_z;" \
            "uniform float aspectRatio;" \
            "void main() " \
            "{ " \
            "  vec3 prepos = gl_Vertex.xyz; " \
            "  prepos.y *= -1.0;" \
            "  vec4 pos = vec4((prepos * model).xy, gl_Vertex.z, gl_Vertex.w);" \
            "  pos.w *= camera_z;" \
            "  pos.x += model[2][0] + view[2][0];" \
            "  pos.y += model[2][1] + view[2][1];" \
            "  pos.x /= aspectRatio;" \
            "  gl_Position = pos; " \
            "  gl_TexCoord[0] = gl_MultiTexCoord0; " \
            "  gl_FrontColor = gl_Color; " \
            "}";

        const std::string fragment = \
            "void main() " \
            "{ " \
            "  gl_FragColor = gl_Color;" \
            "} ";

        S2D_ASSERT(shader.loadFromMemory(vertex, fragment), "Sprite shader failed to load");
    }

    void Renderer::set_uniforms(
        flecs::entity entity, 
        flecs::entity camera, 
        sf::Shader* shader,
        sf::RenderTarget& target)
    {
        const auto* camera_transform = camera.get<Transform>();
        const auto* entity_transform = entity.get<Transform>();
        S2D_ASSERT(camera_transform, "Camera missing transform");
        
        // Model matrix
        sf::Transform model, view;
        model.translate(sf::Vector2f(
            entity_transform->position.x,
            entity_transform->position.y
        ));
        model.scale(sf::Vector2f(1.f, 1.f) * entity_transform->scale);
        model.rotate(sf::degrees(entity_transform->rotation));

        // View matrix
        view.translate(-1.f * sf::Vector2f(
            camera_transform->position.x,
            camera_transform->position.y
        ));
        view.rotate(sf::degrees(camera_transform->rotation));

        // Set the matrices in the shader
        shader->setUniform("aspectRatio", (float)target.getSize().x / (float)target.getSize().y);
        shader->setUniform("camera_z", camera_transform->position.z);
        shader->setUniform("model", sf::Glsl::Mat3(model));
        shader->setUniform("view",  sf::Glsl::Mat3(view));
    }

    template<>
    void 
    Renderer::renderComponent<Sprite>(
        flecs::entity camera, 
        flecs::entity e, 
        sf::RenderTarget& target,
        sf::Shader* shader) const
    {
        if (!shader) shader = &default_sprite->shader;
        Renderer::set_uniforms(e, camera, shader, target);
        shader->setUniform("sprite_size", e.get<Sprite>()->size);

        MeshBuilder<Sprite>::checkAndBuild(e);

        const auto* sprite = e.get<Sprite>();
        S2D_ASSERT(sprite->mesh, "Error generating sprite mesh");

        sf::RenderStates states;
        states.shader = shader;

        if (sprite->texture.size())
        {
            auto* texture = [&]() 
            {
                const auto res = _scene->resources.getResource<sf::Texture>(sprite->texture);
                S2D_ASSERT(res, "Error loading texture");
                return res.value();
            }();
            states.texture = texture;
        }

        target.draw(sprite->mesh->vertices, states);
    }

    template<>
    void 
    Renderer::renderComponent<Tilemap>(
        flecs::entity camera, 
        flecs::entity e, 
        sf::RenderTarget& target,
        sf::Shader* shader) const
    {
        if (!shader) shader = &default_tilemap->shader;
        Renderer::set_uniforms(e, camera, shader, target);

        MeshBuilder<Tilemap>::checkAndBuild(e);

        const auto* tilemap = e.get<Tilemap>();
        S2D_ASSERT(tilemap->mesh, "Error generating tilemap mesh");

        sf::RenderStates states;
        states.shader = shader;

        // If the tilemap has a texture (it should always) set the state
        if (tilemap->spritesheet.texture_name.size())
        {
            auto texture = _scene->resources.getResource<sf::Texture>(tilemap->spritesheet.texture_name);
            S2D_ASSERT(texture, "Error getting texture");
            states.texture = texture.value();
        }

        target.draw(tilemap->mesh->vertices, states);
    }

    template<>
    void 
    Renderer::renderComponent<Text>(
        flecs::entity camera, 
        flecs::entity e, 
        sf::RenderTarget& target,
        sf::Shader* shader) const
    {
        sf::RenderStates states;
        states.shader = shader;
    }

    template<>
    void 
    Renderer::renderComponent<CustomMesh>(
        flecs::entity camera, 
        flecs::entity e, 
        sf::RenderTarget& target,
        sf::Shader* shader) const
    {
        if (!shader) shader = &default_mesh->shader;
        Renderer::set_uniforms(e, camera, shader, target);

        sf::RenderStates states;
        states.shader = shader;

        auto& mesh = e.get_mut<CustomMesh>()->mesh;
        switch (mesh->primitive)
        {
        case Primitive::Lines:     mesh->vertices.setPrimitiveType(sf::PrimitiveType::Lines);     break;
        case Primitive::Points:    mesh->vertices.setPrimitiveType(sf::PrimitiveType::Points);    break;
        case Primitive::Triangles: mesh->vertices.setPrimitiveType(sf::PrimitiveType::Triangles); break;
        case Primitive::Count: return;
        }

        target.draw(mesh->vertices, states);
    }

    Renderer::Renderer(Scene* scene) :
        _scene(scene),
        transforms(
            scene->world.query_builder<const Transform>()
                .order_by<Transform>(
                [](flecs::entity_t e1, const Transform *d1, flecs::entity_t e2, const Transform *d2) 
                {
                    return (d1->position.z > d2->position.z) - (d1->position.z < d2->position.z);
                })
                .build()
        ),
        default_mesh(std::make_unique<DefaultShader<CustomMesh>>()),
        default_sprite(std::make_unique<DefaultShader<Sprite>>()),
        default_tilemap(std::make_unique<DefaultShader<Tilemap>>())
    {   }

    void 
    Renderer::render(
        flecs::entity camera, 
        sf::RenderTarget& target) const
    {
        transforms.each(
            [&](flecs::entity e, const Transform&)
            {
                render(camera, e, target);
            });
    }

    void 
    Renderer::render(
        flecs::entity camera, 
        const std::string& entity,
        sf::RenderTarget& target) const
    {
        auto e = _scene->world.lookup(entity.c_str());
        S2D_ASSERT(e, "Trying to render invalid entity");
        render(camera, e, target);
    }

#define RENDER_COMPONENT(name) if (entity.has<name>()) renderComponent<name>(camera, entity, target, shader)

    void 
    Renderer::render(
        flecs::entity camera, 
        flecs::entity entity,
        sf::RenderTarget& target) const
    {
        S2D_ASSERT(entity.has<Transform>(), "Entity missing transform");
        S2D_ASSERT(camera.has<Camera>(), "Camera missing camera component");

        sf::Shader* shader = nullptr;

        const auto* component = entity.get<ShaderComp>();
        if (component)
        {
            // Set relevant information
            const auto res = _scene->resources.getResource<sf::Shader>(component->name);
            S2D_ASSERT(res, "Error loading shader");
            shader = res.value();

            // Load the textures
            [&]() {
                for (const auto& p : component->textures)
                {
                    switch (p.first)
                    {
                    case TexSource::Renderpass:
                    {
                        S2D_ASSERT(_scene->renderpass->targets.count(p.second), "Renderpass doesn't contain requested resource");
                        const auto& pass = _scene->renderpass->targets.at(p.second);
                        pass->display();
                        shader->setUniform("texture_size", (sf::Vector2f)pass->getSize());
                        shader->setUniform("texture", pass->getTexture());
                        return;
                    }
                    case TexSource::Resources:
                    {
                        const auto resource = _scene->resources.getResource<sf::Texture>(p.second);
                        S2D_ASSERT(resource, "Failed to load resource!");
                        shader->setUniform("texture_size", (sf::Vector2f)resource.value()->getSize());
                        shader->setUniform("texture", *resource.value());
                        return;
                    }
                    default: break;
                    }
                }
            }();
        }

        // Create the model and view matrices
        /*
        // Set the rest of the information needed
        shader->setUniform("target_size", (sf::Vector2f)target.getSize());
        shader->setUniform("camera_size", (sf::Vector2f)camera.get<Camera>()->size);*/

        RENDER_COMPONENT(Sprite);
        RENDER_COMPONENT(Tilemap);
        RENDER_COMPONENT(Text);
        RENDER_COMPONENT(CustomMesh);
    }
}

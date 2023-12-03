#include <Simple2D/Engine/Renderer.hpp>
#include <Simple2D/Engine/Mesh.hpp>
#include <Simple2D/Engine/Core.hpp>

#include <Simple2D/Util/Transform.hpp>
#include <Simple2D/Util/Angle.hpp>

#include <Simple2D/Log/Log.hpp>

namespace S2D::Engine
{
    template<>
    DefaultShader<Graphics::Surface>::DefaultShader()
    {
        const std::string vertex = 
        #include "DefaultShader/flat.vert.glsl"
        ;

        const std::string fragment = 
        #include "DefaultShader/flat.frag.glsl"
        ;
        
        S2D_ASSERT(shader.fromString(vertex, Graphics::Shader::Type::Vertex),   "Sprite vertex shader failed to load");
        S2D_ASSERT(shader.fromString(fragment, Graphics::Shader::Type::Fragment), "Sprite fragment shader failed to load");
        shader.link();
    }

    template<>
    DefaultShader<Sprite>::DefaultShader()
    {
        const std::string vertex = 
        #include "DefaultShader/sprite.vert.glsl"
        ;

        const std::string fragment = 
        #include "DefaultShader/sprite.frag.glsl"
        ;
        
        S2D_ASSERT(shader.fromString(vertex, Graphics::Shader::Type::Vertex),   "Sprite vertex shader failed to load");
        S2D_ASSERT(shader.fromString(fragment, Graphics::Shader::Type::Fragment), "Sprite fragment shader failed to load");
        shader.link();
    }

    template<>
    DefaultShader<Tilemap>::DefaultShader()
    {

    }

    template<>
    DefaultShader<CustomMesh>::DefaultShader()
    {
        const std::string vertex = 
        #include "DefaultShader/custommesh.vert.glsl"
        ;

        const std::string fragment = 
        #include "DefaultShader/custommesh.frag.glsl"
        ;

        S2D_ASSERT(shader.fromString(vertex, Graphics::Shader::Type::Vertex),   "CustomMesh vertex shader failed to load");
        S2D_ASSERT(shader.fromString(fragment, Graphics::Shader::Type::Fragment), "CustomMesh fragment shader failed to load");
        shader.link();
    }

    void Renderer::set_uniforms(
        flecs::entity entity, 
        flecs::entity camera, 
        Graphics::Program* shader,
        Graphics::Surface& target)
    {
        const auto* camera_transform = camera.get<Transform>();
        const auto* entity_transform = entity.get<Transform>();
        S2D_ASSERT(camera_transform, "Camera missing transform");

        Math::Transform model, view;
        model.translate(entity_transform->position);
        model.scale({ entity_transform->scale, entity_transform->scale, entity_transform->scale });
        model.rotate({ 0.f, 0.f, entity_transform->rotation });

        // View matrix
        view.translate(camera_transform->position * -1.f);
        //view.rotate({ 0.f, 0.f, camera_transform->rotation });

        const auto aspectRatio = (float)target.getSize().x / (float)target.getSize().y;

        const auto near = 0.01;
        const auto far = 10000.0;
        const auto t = 1.f / tanf(Util::degrees(camera.get<Camera>()->FOV / 2.f).asRadians());

        Math::Mat4f proj(false);
        proj[0][0] = t / aspectRatio;
        proj[1][1] = t;
        proj[2][2] = -1.f * (far + near) / (far - near);
        proj[3][2] = -2.f * (far * near) / (far - near);
        proj[2][3] = -1.f;

        const auto mat = model.matrix() * view.matrix() * proj;

        // Set the matrices in the shader
        shader->setUniform("MVP", mat);
    }

    template<>
    void 
    Renderer::renderComponent<Sprite>(
        flecs::entity camera, 
        flecs::entity e, 
        Graphics::Surface& target,
        Graphics::Context context) const
    {
        if (!context.program) context.program = &default_sprite->shader;
        Renderer::set_uniforms(e, camera, context.program, target);
        context.program->setUniform("spriteSize", e.get<Sprite>()->size);

        MeshBuilder<Sprite>::checkAndBuild(e);

        const auto* sprite = e.get<Sprite>();
        S2D_ASSERT(sprite->mesh, "Error generating sprite mesh");

        if (sprite->texture.size())
        {
            auto* texture = [&]() 
            {
                const auto res = _scene->resources.getResource<Graphics::Texture>(sprite->texture);
                S2D_ASSERT(res, "Error loading texture");
                return res.value();
            }();
            context.textures.push_back(texture);
        }

        target.draw(sprite->mesh->vertices, context);
    }

    template<>
    void 
    Renderer::renderComponent<Tilemap>(
        flecs::entity camera, 
        flecs::entity e, 
        Graphics::Surface& target,
        Graphics::Context context) const
    {
        if (!context.program) context.program = &default_tilemap->shader;
        Renderer::set_uniforms(e, camera, context.program, target);

        MeshBuilder<Tilemap>::checkAndBuild(e);

        const auto* tilemap = e.get<Tilemap>();
        S2D_ASSERT(tilemap->mesh, "Error generating tilemap mesh");

        // If the tilemap has a texture (it should always) set the state
        if (tilemap->spritesheet.texture_name.size())
        {
            auto texture = _scene->resources.getResource<Graphics::Texture>(tilemap->spritesheet.texture_name);
            S2D_ASSERT(texture, "Error getting texture");
            context.textures.push_back(texture.value());
        }

        target.draw(tilemap->mesh->vertices, context);
    }

    template<>
    void 
    Renderer::renderComponent<Text>(
        flecs::entity camera, 
        flecs::entity e, 
        Graphics::Surface& target,
        Graphics::Context context) const
    {   }

    /*
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
    }*/

    template<>
    void 
    Renderer::renderComponent<CustomMesh>(
        flecs::entity camera, 
        flecs::entity e, 
        Graphics::Surface& target,
        Graphics::Context context) const
    {
        if (!context.program) context.program = &default_mesh->shader;
        Renderer::set_uniforms(e, camera, context.program, target);

        auto& mesh = e.get_mut<CustomMesh>()->mesh;

        target.draw(mesh->vertices, context);
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
        default_tilemap(std::make_unique<DefaultShader<Tilemap>>()),
        default_flat(std::make_unique<DefaultShader<Graphics::Surface>>())
    {   }

    void 
    Renderer::renderQuad(
        Math::Transform& transform, 
        Graphics::Surface& target, 
        const Graphics::Texture* texture) const
    {
        const auto mesh = RawMesh::getQuadMesh();

        Graphics::Context context;
        context.program = &default_flat->shader;
        context.program->setUniform("model", transform.matrix());

        if (texture) context.textures.push_back(texture);

        target.draw(mesh->vertices, context);
    }

    void 
    Renderer::render(
        flecs::entity camera, 
        Graphics::Surface& target) const
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
        Graphics::Surface& target) const
    {
        auto e = _scene->world.lookup(entity.c_str());
        S2D_ASSERT(e, "Trying to render invalid entity");
        render(camera, e, target);
    }

#define RENDER_COMPONENT(name) if (entity.has<name>()) renderComponent<name>(camera, entity, target, context)

    void 
    Renderer::render(
        flecs::entity camera, 
        flecs::entity entity,
        Graphics::Surface& target) const
    {
        S2D_ASSERT(entity.has<Transform>(), "Entity missing transform");
        S2D_ASSERT(camera.has<Camera>(), "Camera missing camera component");

        Graphics::Context context;

        const auto* component = entity.get<ShaderComp>();
        if (component)
        {
            // Set relevant information
            const auto res = _scene->resources.getResource<Graphics::Program>(component->name);
            S2D_ASSERT(res, "Error loading shader");
            context.program = res.value();

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
                        context.textures.push_back(pass->texture());
                        return;
                    }
                    case TexSource::Resources:
                    {
                        const auto resource = _scene->resources.getResource<Graphics::Texture>(p.second);
                        S2D_ASSERT(resource, "Failed to load resource!");
                        context.textures.push_back(resource.value());
                        return;
                    }
                    default: break;
                    }
                }
            }();
        }

        RENDER_COMPONENT(Sprite);
        RENDER_COMPONENT(Tilemap);
        RENDER_COMPONENT(Text);
        RENDER_COMPONENT(CustomMesh);
    }
}

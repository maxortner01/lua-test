#include <Simple2D/Engine/Renderer.hpp>
#include <Simple2D/Engine/Mesh.hpp>
#include <Simple2D/Engine/Core.hpp>
#include <Simple2D/Engine/LuaLib/Input.hpp>

#include <Simple2D/Graphics/Font.hpp>

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
    DefaultShader<Text>::DefaultShader()
    {
        const std::string vertex = 
        #include "DefaultShader/text.vert.glsl"
        ;

        const std::string fragment = 
        #include "DefaultShader/text.frag.glsl"
        ;

        S2D_ASSERT(shader.fromString(vertex,   Graphics::Shader::Type::Vertex),   "Sprite vertex shader failed to load");
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

        const auto model = modelTransform(entity_transform);
        const auto view  = viewMatrix(camera);
        const auto proj  = projectionMatrix(camera);

        const auto mat = model.matrix() * view * proj;

        // Set the matrices in the shader
        shader->setUniform("model", model.matrix());
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
            const auto* texture = [&]() -> const Graphics::Texture*
            {
                const auto res = _scene->resources.getResource<Graphics::Texture>(sprite->texture);
                if (res) return res.value();

                const auto r = _scene->resources.getResource<Graphics::DrawTexture>(sprite->texture);
                S2D_ASSERT_ARGS(r, "No texture or DrawTexture with name %s", sprite->texture.c_str());
                return r.value()->texture();
            }();
            context.textures.push_back(texture);
        }

        /*
        const auto mouse_over = [&]()
        {
            const auto* transform = e.get<Transform>();
            S2D_ASSERT(transform, "Entity missing transform");
            
            const auto size = 0.162; // wtf is happening here?

            auto model = modelTransform(transform).matrix();
            //model.at(1, 1) *= sprite->size.y / sprite->size.x;
            const auto view = viewMatrix(camera);
            const auto proj = projectionMatrix(camera);
            const auto inv_MVP = Math::inverse(model * view * proj).transpose();

            const auto pos   = Input::mouse_position;
            const auto pos4d = Math::Vec4f{ pos.x, pos.y, 0.f, 0.f };
            const auto inv = inv_MVP * pos4d.matrix();
            const auto t_pos = Math::Vec2f(inv.at(0, 0), inv.at(1, 0) * sprite->size.x / sprite->size.y);

            if (t_pos.x >= -1.f * size && t_pos.x <= size && t_pos.y >= -1.f * size && t_pos.y <= size)
                return true;
            return false;
        }();

        if (mouse_over && !moused_over->count(e.raw_id()))
        {
            moused_over->insert(std::pair(e.raw_id(), 0));
            // call mouse enter function
            if (e.has<Script>())
            {
                const auto* script = e.get<Script>();
                for (auto& runtime : script->runtime)
                {
                    if (runtime.second) // if it's been initialized
                    {
                        const auto res = runtime.first->runFunction<>("OnMouseEnter");
                        if (!res && res.error().code() != Lua::Runtime::ErrorCode::NotFunction)
                            Log::Logger::instance("engine")->error("Error in OnMouseEnter(): {}", res.error().message());
                    }
                }
            }
        }
        if (!mouse_over && moused_over->count(e.raw_id()))
        {
            moused_over->erase(e.raw_id());
            // call mouse leave function
            if (e.has<Script>())
            {
                const auto* script = e.get<Script>();
                for (auto& runtime : script->runtime)
                {
                    if (runtime.second) // if it's been initialized
                    {
                        const auto res = runtime.first->runFunction<>("OnMouseLeave");
                        if (!res && res.error().code() != Lua::Runtime::ErrorCode::NotFunction)
                            Log::Logger::instance("engine")->error("Error in OnMouseEnter(): {}", res.error().message());
                    }
                }
            }
        }*/

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

    void 
    Renderer::renderText(
        const Math::Transform& transform, 
        Graphics::Surface& target, 
        const std::string& text, 
        Graphics::Font* font, 
        uint32_t pixel_height) const
    {
        S2D_ASSERT(font, "Must have font to render text");
        
        Graphics::VertexArray vao;
        const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 1
        };
        vao.uploadIndices(indices);

        const auto scale = transform.getScale();
        auto pos = Math::Vec2f(0, 0);
        for (const auto& c : text)
        {
            const auto& glyph = font->getCharacter(c, pixel_height);
            
            float xpos = pos.x + glyph.bearing.x / (float)pixel_height * scale.x;
            float ypos = pos.y - (glyph.size.y - glyph.bearing.y) / (float)pixel_height * scale.y * -1.f;

            float w = glyph.size.x / (float)pixel_height * scale.x;
            float h = glyph.size.y / (float)pixel_height * scale.y * -1.f;

            const std::vector<Graphics::Vertex> vertices = {
                Graphics::Vertex{
                    .position = Math::Vec3f(xpos, ypos, 0.f), .color = Graphics::Color(255, 255, 255, 255), .texCoords = Math::Vec2f(0, 1)
                },
                Graphics::Vertex{
                    .position = Math::Vec3f(xpos + w, ypos, 0.f), .color = Graphics::Color(255, 255, 255, 255), .texCoords = Math::Vec2f(1, 1)
                },
                Graphics::Vertex{
                    .position = Math::Vec3f(xpos, ypos + h, 0.f), .color = Graphics::Color(255, 255, 255, 255), .texCoords = Math::Vec2f(0, 0)
                },
                Graphics::Vertex{
                    .position = Math::Vec3f(xpos + w, ypos + h, 0.f), .color = Graphics::Color(255, 255, 255, 255), .texCoords = Math::Vec2f(1, 0)
                }
            };
            vao.upload(vertices, true);
            
            Graphics::Context context;
            context.program = &default_text->shader;
            context.textures.push_back(glyph.texture.get());

            context.program->setUniform("model", transform.matrix());
            target.draw(vao, context);

            pos.x += (float)(glyph.advance >> 6) / (float)pixel_height * scale.x;
        }
    }

    template<>
    void 
    Renderer::renderComponent<Text>(
        flecs::entity camera, 
        flecs::entity e, 
        Graphics::Surface& target,
        Graphics::Context context) const
    {   
        if (!context.program) context.program = &default_text->shader;
        Renderer::set_uniforms(e, camera, context.program, target);
    }

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
        default_text(std::make_unique<DefaultShader<Text>>()),
        default_flat(std::make_unique<DefaultShader<Graphics::Surface>>())//,
        //moused_over(std::make_unique<std::unordered_map<flecs::id_t, int>>())
    {   }

    void 
    Renderer::renderQuad(
        const Math::Transform& transform, 
        Graphics::Surface& target, 
        std::optional<QuadInfo> info) const
    {
        const auto mesh = RawMesh::getQuadMesh();

        Graphics::Context context;
        context.program = &default_flat->shader;
        context.program->setUniform("model", transform.matrix());
        
        Camera camera;
        camera.projection = Projection::Orthographic;
        camera.size = target.getSize();
        context.program->setUniform("proj", projectionMatrix(camera));

        if (info.has_value())
        {
            context.depth_test = info.value().depth_test;
            if (info.value().texture) context.textures.push_back(info.value().texture);
        }

        target.draw(mesh->vertices, context);
    }

    void 
    Renderer::render(
        flecs::entity camera, 
        Graphics::Surface& target,
        Graphics::Program* shader) const
    {
        transforms.each(
            [&](flecs::entity e, const Transform&)
            {
                render(camera, e, target, shader);
            });
    }

    void 
    Renderer::render(
        flecs::entity camera, 
        const std::string& entity,
        Graphics::Surface& target,
        Graphics::Program* shader) const
    {
        auto e = _scene->world.lookup(entity.c_str());
        S2D_ASSERT(e, "Trying to render invalid entity");
        render(camera, e, target, shader);
    }

#define RENDER_COMPONENT(name) if (entity.has<name>()) renderComponent<name>(camera, entity, target, context)

    void 
    Renderer::render(
        flecs::entity camera, 
        flecs::entity entity,
        Graphics::Surface& target,
        Graphics::Program* shader) const
    {
        S2D_ASSERT(entity.has<Transform>(), "Entity missing transform");
        S2D_ASSERT(camera.has<Camera>(), "Camera missing camera component");

        Graphics::Context context;
        context.program = shader;

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

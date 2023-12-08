#pragma once

#include "../Util/Transform.hpp"

#include "Components.hpp"

namespace S2D::Graphics
{
    struct Font;
}

namespace S2D::Engine
{
    struct Scene;

    template<typename T>
    struct DefaultShader
    {
        DefaultShader();

        Graphics::Program shader;
    };

    struct Renderer
    {
        Renderer(Scene* scene);

        struct QuadInfo
        {
            const Graphics::Texture* texture = nullptr;
            bool depth_test = true;
        };

        void renderQuad(const Math::Transform& transform, Graphics::Surface& target, std::optional<QuadInfo> info = std::nullopt) const;
        void renderText(const Math::Transform& transform, Graphics::Surface& target, const std::string& text, Graphics::Font* font, uint32_t pixel_height = 16) const;

        void render(flecs::entity camera, Graphics::Surface& target, Graphics::Program* shader = nullptr) const;
        void render(flecs::entity camera, const std::string& entity, Graphics::Surface& target, Graphics::Program* shader = nullptr) const;
        void render(flecs::entity camera, flecs::entity entity, Graphics::Surface& target, Graphics::Program* shader = nullptr) const;

        template<typename _Component>
        void renderComponent(flecs::entity camera, flecs::entity e, Graphics::Surface& target, Graphics::Context context) const;

    private:
        Scene* _scene;
        flecs::query<const Transform> transforms;

        static void set_uniforms(flecs::entity entity, flecs::entity camera, Graphics::Program* shader, Graphics::Surface& target);

        std::unique_ptr<DefaultShader<Text>> default_text;
        std::unique_ptr<DefaultShader<Sprite>> default_sprite;
        std::unique_ptr<DefaultShader<Tilemap>> default_tilemap;
        std::unique_ptr<DefaultShader<CustomMesh>> default_mesh;
        std::unique_ptr<DefaultShader<Graphics::Surface>> default_flat;
    };
}

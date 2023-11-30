#pragma once

#include "Components.hpp"

namespace S2D::Engine
{
    struct Scene;

    template<typename T>
    struct DefaultShader
    {
        DefaultShader();

        sf::Shader shader;
    };

    struct Renderer
    {
        Renderer(Scene* scene);

        void render(flecs::entity camera, sf::RenderTarget& target) const;
        void render(flecs::entity camera, const std::string& entity, sf::RenderTarget& target) const;
        void render(flecs::entity camera, flecs::entity entity, sf::RenderTarget& target) const;

        template<typename _Component>
        void renderComponent(flecs::entity camera, flecs::entity e, sf::RenderTarget& target, sf::Shader* shader) const;

    private:
        Scene* _scene;
        flecs::query<const Transform> transforms;

        static void set_uniforms(flecs::entity entity, flecs::entity camera, sf::Shader* shader, sf::RenderTarget& target);

        std::unique_ptr<DefaultShader<Sprite>> default_sprite;
        std::unique_ptr<DefaultShader<Tilemap>> default_tilemap;
        std::unique_ptr<DefaultShader<CustomMesh>> default_mesh;
    };
}
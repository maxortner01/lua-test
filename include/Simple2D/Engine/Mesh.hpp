#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include <fcl/fcl.h>
#include <flecs.h>

namespace S2D::Engine
{
    struct RawMesh
    {
        sf::VertexArray vertices;
    };

    struct CollisionMesh
    {
        using Model = fcl::BVHModel<fcl::OBBRSSf>;
        std::vector<fcl::Vector3f> vertices;
        std::vector<fcl::Triangle> triangles;
        std::shared_ptr<Model> model;
    };

    struct Mesh
    {
        flecs::entity entity;
        int32_t component_id;
        RawMesh mesh;
        std::shared_ptr<CollisionMesh> collision;

        //inline static std::unordered_map<flecs::id_t, Mesh*> meshes;

        Mesh(flecs::entity _entity);
        Mesh(const Mesh&) = delete;
        Mesh(Mesh&&) = delete;

        virtual ~Mesh();

        virtual void build() = 0;
    };

    // Unique mesh contains its own vertex data
    // Instanced mesh contains a shared pointer to vertex data
    template<typename Data>
    struct TypeMesh : Mesh
    {
        TypeMesh(flecs::entity e);
        void build() override;
    };
}
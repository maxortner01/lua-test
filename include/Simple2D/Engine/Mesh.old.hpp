#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include <fcl/fcl.h>
#include <flecs.h>

namespace S2D::Engine
{
    // Here's what needs to happen
    // Colliders have a unique_ptr to a CollisionMesh
    // Sprites and Tilemaps have a unique_ptr to a Mesh
    // Mesh's have a virtual method that returns a reference to a RawMesh called getGeometry()
    // Instanced Meshs override this and pulls the RawMesh from a common source
    // Unique Meshs override this and pulls it from local source


    struct RawMesh
    {
        sf::VertexArray vertices;
    };

    struct CollisionMesh
    {
        using Model = fcl::BVHModel<fcl::OBBRSSf>;
        std::vector<fcl::Vector3f> vertices;
        std::vector<fcl::Triangle> triangles;
        std::unique_ptr<Model> model;
    };

    /*
    struct Mesh
    {
        flecs::entity entity;
        int32_t component_id;
        std::shared_ptr<RawMesh> raw_mesh;
        std::shared_ptr<CollisionMesh> collision;

        Mesh(flecs::entity _entity);
        ~Mesh();
    };
    */  
    // Unique mesh contains its own vertex data
    // Instanced mesh contains a shared pointer to vertex data

    // Need to generate collision mesh for each instance that scales the mesh
    // geometry correctly
    // When a transform's scale is changed, the collision mesh needs to be regenerated :(
    // That, or we generate collision meshes every frame for each mesh already there... Not good, much worse

    // Down the line, some mesh optimizations are REQUIRED, as the tilemap meshes are 100s of times larger
    // in memory than they need to be

    /*
    template<typename Data>
    struct MeshBuilder
    {
        static Mesh get(flecs::entity e);
    };*/
}
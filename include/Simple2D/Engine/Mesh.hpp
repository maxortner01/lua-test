#pragma once

#include "../Graphics.hpp"

#include <fcl/fcl.h>
#include <flecs.h>

namespace S2D::Engine
{
    enum class Primitive
    {
        Points, Triangles, Lines, Count
    };

    const char* operator*(Primitive p);

    struct RawMesh
    {
        Primitive primitive = Primitive::Triangles;
        Graphics::VertexArray vertices;
        
        static std::shared_ptr<RawMesh> getQuadMesh();
    };

    struct CollisionMesh
    {
        using Model = fcl::BVHModel<fcl::OBBRSSf>;
        //void* fcl_collision_data;
        std::vector<fcl::Vector3f> vertices;
        std::vector<fcl::Triangle> triangles;
        std::shared_ptr<Model> model;
    };

    template<typename T>
    struct MeshBuilder
    {
        static void checkAndBuild(flecs::entity e);
    };
}
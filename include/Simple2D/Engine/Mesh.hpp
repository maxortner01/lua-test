#pragma once

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

    template<typename T>
    struct MeshBuilder
    {
        static void checkAndBuild(flecs::entity e);
    };
}
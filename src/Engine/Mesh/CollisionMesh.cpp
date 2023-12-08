#include <Simple2D/Engine/Mesh.hpp>

#include <fcl/fcl.h>

namespace S2D::Engine
{

struct CollisionFCL
{
    using Model = fcl::BVHModel<fcl::OBBRSSf>;
    std::vector<fcl::Vector3f> vertices;
    std::vector<fcl::Triangle> triangles;
    std::shared_ptr<Model> model;
};

}
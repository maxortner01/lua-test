#include <Simple2D/Engine/Core.hpp>
#include <Simple2D/Log/Log.hpp>

namespace S2D::Engine
{

void Core::collide(Scene* scene)
{
    auto& logger = Log::Logger::instance("engine");
    auto& world = scene->world;
    auto tilemap_filter = world.filter<const Transform, const Collider>(); 

    std::unordered_map<flecs::id_t, std::unique_ptr<fcl::CollisionObjectf>> map;
    tilemap_filter.each(
        [&](
            flecs::entity    entity,
            const Transform& transform,
            const Collider&  collider)
        {
            if (!collider.mesh || (collider.mesh && !collider.mesh->model)) return;

            S2D_ASSERT(!map.count(entity.raw_id()), "Something went wrong");
            auto transform_matrix = fcl::Transform3f::Identity();
            transform_matrix.translation() = fcl::Vector3f(transform.position.x / transform.scale, transform.position.y / transform.scale, 0);
            map.insert(std::pair(
                entity.raw_id(),
                std::make_unique<fcl::CollisionObjectf>(collider.mesh->model, transform_matrix)
            ));
            map.at(entity.raw_id())->setUserData((void*)entity.raw_id());
        });

    scene->colliders.each(
        [&](
            flecs::entity    entity_a,
            const Collider&  collider_a, 
            /***/ Transform& transform_a,
            /***/ Rigidbody& rigid_body_a)
        {
            if (!map.count(entity_a.raw_id())) return;
            auto& object_a = map.at(entity_a.raw_id());

            tilemap_filter.each([&](
                flecs::entity    entity_b,
                const Transform& transform_b,
                const Collider&  collider_b)
            {
                if (entity_a == entity_b || !map.count(entity_b.raw_id())) return;
                auto& object_b = map.at(entity_b.raw_id());

                fcl::CollisionRequest<float> request{0};
                request.enable_contact = true;
                request.num_max_contacts = 20;

                fcl::CollisionResult<float>  result;
                fcl::collide(object_a.get(), object_b.get(), request, result);

                if (result.isCollision())
                {
                    std::vector<std::pair<float, sf::Vector2f>> depth_normal;

                    logger->info("Collision occured with {} contact points.", result.numContacts());
                    for (uint32_t i = 0; i < result.numContacts(); i++)
                    {
                        const auto& contact = result.getContact(i);

                        const auto& vec   = contact.normal;
                        const auto& depth = contact.penetration_depth;
                        const auto normal = sf::Vector2f(-1.f * vec[0], -1.f * vec[1]);

                        logger->info("Contact point {}: o1 = {}, o2 = {}, normal = ({}, {}), depth = {}", i, 
                            (void*)contact.o1, 
                            (void*)contact.o2,
                            normal.x, normal.y,
                            depth);

                        if (normal.length() && depth && depth < 1.f) depth_normal.push_back(std::pair(depth, normal));
                    }

                    const auto dot = [&]()
                    {
                        sf::Vector2f r;
                        for (const auto& p : depth_normal)
                            r += p.first * p.second;
                        return r * (1.f / depth_normal.size());
                    }();

                    if (!dot.length()) return;

                    logger->info("Computed normal = ({}, {})", dot.x, dot.y);

                    transform_a.position += sf::Vector3f(dot.x, dot.y, 0);
                    rigid_body_a.velocity = rigid_body_a.velocity - 2.f * (rigid_body_a.velocity.dot(dot.normalized())) * dot.normalized();
                }
            });
        });
}

}
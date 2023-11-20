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
                request.num_max_contacts = std::numeric_limits<int>::max();

                fcl::CollisionResult<float>  result;
                fcl::collide(object_b.get(), object_a.get(), request, result);

                if (result.isCollision())
                {
                    std::vector<std::pair<float, sf::Vector2f>> depth_normal;

                    std::vector<sf::Vertex> lines;

                    logger->info("Collision occured with {} contact points.", result.numContacts());
                    for (uint32_t i = 0; i < result.numContacts(); i++)
                    {
                        const auto& contact = result.getContact(i);

                        // Debug draw contact points
                        {
                            sf::RectangleShape rect;
                            rect.setSize({ 5.f, 5.f });
                            rect.setOrigin(rect.getSize() / 2.f);

                            auto index = collider_b.mesh->triangles[contact.b1];

                            bool found = false;
                            for (uint32_t j = 0; j < 3; j++)
                            {
                                auto point = collider_b.mesh->vertices[index[j]];
                                if (point[0] + transform_b.position.x / transform_b.scale == contact.pos[0] &&
                                    point[1] + transform_b.position.y / transform_b.scale == contact.pos[1])
                                    {
                                        found = true;
                                        break;
                                    }
                            }
                            
                            if (found) continue;
                                
                            rect.setFillColor(sf::Color::White);
                            rect.setPosition(sf::Vector2f(contact.pos[0], contact.pos[1]));
                            window.draw(rect);

                            lines.push_back(sf::Vertex(rect.getPosition(), sf::Color::Green));
                            lines.push_back(sf::Vertex(rect.getPosition() + sf::Vector2f(contact.normal[0], contact.normal[1]) * 5.f * contact.penetration_depth, sf::Color::Green));
                        }

                        const auto& vec   = contact.normal;
                        const auto& depth = contact.penetration_depth;
                        const auto normal = sf::Vector2f(-1.f * vec[0], -1.f * vec[1]);

                        if (normal.length() && depth && depth < 1.f) depth_normal.push_back(std::pair(depth, normal));
                    }

                    auto dot = [&]()
                    {
                        sf::Vector2f r;
                        for (const auto& p : depth_normal)
                            r += p.first * p.second * 2.2f;
                        return r * (-1.f / depth_normal.size());
                    }();

                    lines.push_back(sf::Vertex(sf::Vector2f(transform_a.position.x, transform_a.position.y) / transform_a.scale, sf::Color::Yellow));
                    lines.push_back(sf::Vertex(sf::Vector2f(transform_a.position.x, transform_a.position.y) / transform_a.scale + dot * 10.f, sf::Color::Yellow));

                    sf::VertexArray array(sf::PrimitiveType::Lines, lines.size());
                    for (uint32_t i = 0; i < lines.size(); i++)
                        array[i] = lines[i];
                    window.draw(array);

                    // World space and pixel space are the same, so we want to neglect any collisions that are less
                    // than a pixel deep, otherwise we get caught up on too much
                    if (dot.length() <= 1e-1 || std::isnan(dot.x) || std::isnan(dot.y)) return;

                    logger->info("Computed normal = ({}, {})", dot.x, dot.y);
                    
                    // Still not quite right... sometimes, it will invert the direction of the velocity as opposed to 
                    // reflecting it... not quite sure *why* or *when* this happens.
                    const auto e_loss = 1.f;
                    transform_a.position += sf::Vector3f(dot.x, dot.y, 0) * 1.5f;
                    rigid_body_a.velocity = e_loss * (rigid_body_a.velocity - 2.f * (rigid_body_a.velocity.dot(dot.normalized())) * dot.normalized());
                }
            });
        });
}

}

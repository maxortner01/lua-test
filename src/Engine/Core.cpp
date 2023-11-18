#include <Simple2D/Engine.hpp>
#include <Simple2D/Def.hpp>

#include <vector>
#include <algorithm>

namespace S2D::Engine
{

Scene::Scene() :
    scripts(
        world.query_builder<Script>().build()
    ),
    transforms(
        world.query_builder<const ComponentData<Name::Transform>>()
            .order_by<ComponentData<Name::Transform>>(
            [](flecs::entity_t e1, const ComponentData<Name::Transform> *d1, flecs::entity_t e2, const ComponentData<Name::Transform> *d2) 
            {
                return (d1->position.z > d2->position.z) - (d1->position.z < d2->position.z);
            })
            .build()
    ),
    colliders(
        world.query_builder<ComponentData<Name::Collider>>().build()
    )
{

}

Scene* Core::getTopScene()
{
    return _scenes.top();
}

#define ITEM(x) case sf::Keyboard::Key::x: return #x[0];
char getSFMLKey(const sf::Keyboard::Key& key)
{
    switch (key)
    {
    ITEM(W)
    ITEM(A)
    ITEM(S)
    ITEM(D)
    default: return ' ';
    }
}

void Core::run()
{
    auto tick = std::chrono::high_resolution_clock::now();

    while (window.isOpen() && _scenes.size())
    {
        sf::Event event;

        // Change all KeyState::Pressed to KeyState::Down
        // Erase all KeyState::Released
        std::vector<char> released;
        for (auto& p : Input::global_state)
        {
            if (p.second == Input::KeyState::Press) p.second = Input::KeyState::Down;
            if (p.second == Input::KeyState::Release) released.push_back(p.first);
        }

        for (const auto& c : released) Input::global_state.erase(c);

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed)
            {
                const auto code = getSFMLKey(event.key.code);
                if (!Input::global_state.count(code)) Input::global_state.insert(std::pair(code, Input::KeyState::Press));
            }
            else if (event.type == sf::Event::KeyReleased)
            {
                const auto code = getSFMLKey(event.key.code);
                if (Input::global_state.count(code)) Input::global_state.at(code) = Input::KeyState::Release;
            }
        }

    get_top_scene:
        if (!_scenes.size()) break;
        auto* top_scene = getTopScene();
        if (!top_scene->running)
        {
            delete top_scene;
            _scenes.pop();
            goto get_top_scene;
        }

        // We execute all the scripts
        // THEN the other registered systems run with world.progress()
        // THEN the draw method is called
        
        auto& world = top_scene->world;
        top_scene->scripts.each([&](flecs::entity e, Script& script)
        {
            if (!e.is_alive() || e.has<Dead>()) return;

            // Execute the update function
            auto ent = Engine::Entity().asTable();
            ent.set("entity", e.raw_id());
            ent.set("good", true);
            ent.set("world", (uint64_t)world.c_ptr()); // Currently hacky way to store a pointer (must be considered an int64)

            auto _world = Engine::World().asTable();
            _world.set("world", (uint64_t)world.c_ptr());
            _world.set("good", true);

            for (auto& script : script.runtime)
            {
                S2D_ASSERT(script.first, "Script runtime is null!");
                if (!script.second) { script.first->template runFunction<>("Start", _world, ent); script.second = true; }
                script.first->template runFunction<>("Update", _world, ent);
            }

            // Check if it has a collider component and execute the collision function
        });

        if (world.count<Dead>())
        {
            std::vector<flecs::entity> dead_entities;
            dead_entities.reserve(world.count<Dead>());
            world.filter<Dead>().each([&](flecs::entity e, Dead d)
                { dead_entities.push_back(e); }
            );
            for (auto& e : dead_entities) e.destruct();
        }

        window.clear();

        world.progress();

        collide(top_scene);
        render(top_scene);

        top_scene->draw(window);

        window.display();

        auto now = std::chrono::high_resolution_clock::now();
        Time::dt = std::chrono::duration_cast<std::chrono::microseconds>(now - tick).count() / 1e6;
        tick = now;
    }
}

Core::Core(const Application& app) :
    window(
        sf::VideoMode(app.size),
        app.name)
{   }

Core::~Core()
{
    while (_scenes.size()) { delete _scenes.top(); _scenes.pop(); }
}

void Core::collide(Scene* scene)
{
    auto& world = scene->world;
    auto entity_filter = world.filter<
        const ComponentData<Name::Collider>, 
        ComponentData<Name::Rigidbody>>();

    entity_filter.each(
        [&](
            flecs::entity main_e, 
            const ComponentData<Name::Collider>&  collider_a, 
            /***/ ComponentData<Name::Rigidbody>& rigid_body)
        {
            scene->colliders.each(
                [&](
                    flecs::entity collider_e, 
                    ComponentData<Name::Collider>& collider_b)
                {
                    if (main_e.raw_id() == collider_e.raw_id()) return;
                    
                    // Check collider_a to collider_b
                    //   First, check the AABB, if not: return
                    //   If collision has happened, 
                    //     check if main_e has a script,
                    //       if true: call the collide function in the script
                    //     edit the rigid_body.velocity to move the object away
                });
        });
    
}

void Core::render(Scene* scene)
{   
    scene->transforms.each(
        [&](flecs::entity e, const ComponentData<Name::Transform>& transform)
        {
            /* Render Sprites */
            const auto* sprite = e.get<const ComponentData<Name::Sprite>>();
            if (sprite)
            {
                const sf::Texture* texture = nullptr;
                if (sprite->texture.size())
                    texture = scene->resources.getResource<sf::Texture>(sprite->texture).value();

                sf::RectangleShape rect;
                rect.setSize((sf::Vector2f)sprite->size);
                rect.setOrigin(rect.getSize() / 2.f);
                rect.setFillColor(sf::Color::White);
                rect.setPosition(sf::Vector2f((int)transform.position.x, (int)transform.position.y));
                rect.rotate(sf::degrees(transform.rotation));
                
                if (texture) rect.setTexture(texture);
                
                window.draw(rect);
            }

            /* Render Text */
            const auto* text = e.get<const ComponentData<Name::Text>>();
            if (text)
            {
                auto* font = scene->resources.getResource<sf::Font>(text->font).value();
                S2D_ASSERT(font, "Font is null");

                int max_height = std::numeric_limits<int>::min();
                for (const auto& c : text->string)
                {
                    const auto height = font->getGlyph('c', text->character_size, false).bounds.height;
                    if (height > max_height) max_height = height;
                }

                sf::Text t(*font, text->string);
                t.setPosition(sf::Vector2f(
                    (int)transform.position.x,
                    (int)transform.position.y
                ));
                t.setOrigin(sf::Vector2f(
                    (int)t.getLocalBounds().left /*+ (int)(t.getLocalBounds().width / 2.f)*/,
                    (int)t.getLocalBounds().top + (int)(max_height / 2.f)
                ));
                t.setCharacterSize((unsigned int)text->character_size);
                t.setFillColor(sf::Color::White);
                window.draw(t);
            }

            /* Render Tilemap */
            const auto* tilemap = e.get<const ComponentData<Name::Tilemap>>();
            if (tilemap)
            {
                const auto& map = tilemap->tiles.map;
                
                // Initialize the vertex array and render states
                const auto total_tiles = [&]()
                {
                    uint32_t count = 0;
                    for (const auto& p : map)
                        for (const auto& t : p.second.first)
                            count++;
                    return count;
                }();

                uint32_t iterator = 0;
                sf::VertexArray vertices(sf::PrimitiveType::Triangles, total_tiles * 6);
                sf::RenderStates states;

                // If the tilemap has a texture (it should always) set the state
                if (tilemap->spritesheet.texture_name.size())
                {
                    auto texture = scene->resources.getResource<sf::Texture>(tilemap->spritesheet.texture_name);
                    S2D_ASSERT(texture, "Error getting texture");
                    states.texture = texture.value();
                }

                for (const auto& p : map)
                {
                    for (const auto& t : p.second.first)
                    {
                        // Extract the coordinate info from the key
                        const int32_t mask = 0xFFFF0000;
                        int16_t x = ((t.first & mask) >> 16);
                        int16_t y = (t.first & (~mask));
                        const auto position = sf::Vector2f(
                            x * tilemap->tilesize.x * transform.scale + transform.position.x,
                            y * tilemap->tilesize.y * transform.scale + transform.position.y
                        );

                        // Construct the texture rectangle from the tile information
                        sf::IntRect tex_rect;
                        tex_rect.width  = tilemap->tilesize.x;
                        tex_rect.height = tilemap->tilesize.y;
                        tex_rect.left = t.second.texture_coords.x * tilemap->tilesize.x;
                        tex_rect.top  = t.second.texture_coords.y * tilemap->tilesize.y;

                        // Forming a quad from two triangles
                        const sf::Vector2f offsets[] = {
                            { -0.5f,  0.5f },
                            {  0.5f, -0.5f },
                            { -0.5f, -0.5f },

                            {  0.5f, -0.5f },
                            { -0.5f,  0.5f },
                            {  0.5f,  0.5f }
                        };

                        const sf::Vector2i tex_coords[] = {
                            { 0, 1 },
                            { 1, 0 },
                            { 0, 0 },

                            { 1, 0 },
                            { 0, 1 },
                            { 1, 1 }
                        };
                        
                        // Construct the vertices of the quad
                        for (uint8_t i = 0; i < 6; i++)
                        {
                            auto& vertex = vertices[iterator++];
                            vertex.position.x = offsets[i].x * tilemap->tilesize.x * transform.scale + position.x;
                            vertex.position.y = offsets[i].y * tilemap->tilesize.y * transform.scale + position.y;
                            vertex.color = sf::Color::White;  
                            vertex.texCoords.x = tex_rect.left + tex_rect.width  * tex_coords[i].x;
                            vertex.texCoords.y = tex_rect.top + tex_rect.height * tex_coords[i].y;  
                        }
                    }
                }
                
                // Once it's all generated, draw
                window.draw(vertices, states);
            }
        }
    );
}

}

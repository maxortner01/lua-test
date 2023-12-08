#include <Simple2D/Lua.hpp>
#include <Simple2D/Engine.hpp>

#include <sstream>
#include <iostream>
#include <chrono>
#include <numeric>

#include <flecs.h>

using namespace S2D;

#ifndef SOURCE_DIR
#define SOURCE_DIR "."
#endif

struct MainScene : Engine::LuaScene
{
    MainScene(const std::string& filename) :
        LuaScene(filename)
    {   }

    void poststart() override
    {
        auto e = world.lookup("Mesh");
        S2D_ASSERT(e, "Entity not real");
        S2D_ASSERT(e.has<Engine::CustomMesh>(), "Entity missing mesh component");
        auto* mesh = e.get_mut<Engine::CustomMesh>();

        e.get_mut<Engine::ShaderComp>()->textures.push_back(std::pair(
            Engine::TexSource::Renderpass,
            "MowCamera"
        ));

        if (!mesh->mesh) mesh->mesh = std::make_unique<Engine::RawMesh>();
        //mesh->mesh->primitive = Engine::Primitive::Points;
        mesh->mesh->vertices.setDrawType(Graphics::VertexArray::DrawType::Points);

        const auto* image = [&]()
        {
            const auto res = resources.getResource<Graphics::Image>("mask");
            S2D_ASSERT(res, "Mask image missing");
            return res.value();
        }();

        // Random number between -1 and 1
        const auto random = []() 
        {
            return (double)rand() / (double)std::numeric_limits<int>::max();
        };

        const auto point_per_pixel = 3U;
        std::vector<Graphics::Vertex> vertices;
        for (uint32_t y = 0; y < image->getSize().y; y++)
        {
            for (uint32_t x = 0; x < image->getSize().x; x++)
            {
                auto color = image->read({ x, y });
                for (uint32_t i = 0; i < point_per_pixel; i++)
                {
                    if (abs(random()) > color.a ) continue;

                    Graphics::Vertex vertex;
                    vertex.position = Math::Vec3f(
                        (x + random() * 0.9 - (float)image->getSize().x / 2.f) / (float)image->getSize().x,
                        (y + random() * 0.9 - (float)image->getSize().y / 2.f) / (float)image->getSize().y,
                        0.f
                    );
                    vertex.color = Graphics::Color(255, 0, 0, 255);
                    vertices.push_back(vertex);
                }
            }
        }

        mesh->mesh->vertices.upload(vertices);
    }

    void update() override
    {
        auto camera = world.lookup("MowCamera");
        auto* shader = resources.getResource<Graphics::Program>("grass_shader").value();
        shader->setUniform("camera_view", Engine::viewMatrix(camera));
        shader->setUniform("camera_proj", Engine::projectionMatrix(camera));

        auto player = world.lookup("PlayerTop");
        const auto* rigid = player.get<Engine::Rigidbody>();
        auto* bottom_shader = resources.getResource<Graphics::Program>("bottom").value();
        bottom_shader->setUniform("velocity", Math::Vec2f(rigid->velocity.x, rigid->velocity.y));
    }

    void constructPass(Engine::RenderpassBuilder& builder) override
    {
        using namespace S2D::Engine;

        Log::Logger::instance("engine")->info("Constructing pass");

        builder.resource<Resource::Surface>({ "UI", { 1280, 720 } });

        builder.command<Command::BindSurface>({ "MowCamera" })
            .command<Command::RenderEntity>({ "PlayerBottom", "MowCamera", "bottom" });

        builder.command<Command::BindSurface>({ "MainCamera" })
            .command<Command::Clear>({ { 75, 50, 50, 255 }, Graphics::LayerType::Both })
            .command<Command::RenderEntities>({ "MainCamera" });
            //.command<Command::BlitSurface>({ { 0.f, 0.f }, { 1280.f, 720.f } });

        builder.command<Command::BindSurface>({ "UI" })
            .command<Command::Clear>({ { 0, 0, 0, 0 }, Graphics::LayerType::Both })
            .command<Command::RenderUI>({ loadRuntime(SOURCE_DIR "/scripts/ui.lua", world) })
            .command<Command::BlitSurface>({ { 0.f, 0.f }, { 1280.f, 720.f } }); // need to be able to render without depth
    }
};

struct App : Engine::Application
{
    App(const Math::Vec2u& size, const std::string& name) : Application(size)
    {   
        this->name = name;
    }

    void start(Engine::Core& core) override
    {
        core.emplaceScene<MainScene>(SOURCE_DIR "/scripts/new_test.lua");
    }
};

Engine::Application* getApplication()
{
    /* Extract window size info from a config script */
    auto window = [&]()
    {
        Lua::Runtime runtime(SOURCE_DIR "/scripts/config.lua");
        auto res = runtime.getGlobal<Lua::Table>("Window");
        S2D_ASSERT(res, "Error loading the window information");
        return res.value();
    }();

    const auto size = [&]()
    {
        const auto& size = window.get<Lua::Table>("size");
        return Math::Vec2u(
            size.get<Lua::Number>("width"),
            size.get<Lua::Number>("height")
        );
    }();

    const auto& name = window.get<Lua::String>("title");
    Engine::Script::SourceDir = std::string(SOURCE_DIR);

    return new App(size, name);
}

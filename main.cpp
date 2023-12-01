#include <Simple2D/Graphics.hpp>
#include <Simple2D/Util/Matrix.hpp>
#include <Simple2D/Log/Log.hpp>

using namespace S2D;

#ifndef SOURCE_DIR
#define SOURCE_DIR "."
#endif

int main()
{   
    Graphics::DrawWindow window({ 1280, 720 }, "Hello");

    auto& logger = Log::Logger::instance("window");
    
    Graphics::Program program;
    S2D_ASSERT(program.fromFile(SOURCE_DIR "/shaders/new.vert.glsl", Graphics::Shader::Type::Vertex),   "Error loading vertex shader"  );
    S2D_ASSERT(program.fromFile(SOURCE_DIR "/shaders/new.frag.glsl", Graphics::Shader::Type::Fragment), "Error loading fragment shader");
    program.link();

    Graphics::Program flat;
    S2D_ASSERT(flat.fromFile(SOURCE_DIR "/shaders/flat.vert.glsl", Graphics::Shader::Type::Vertex),   "Error loading vertex shader"  );
    S2D_ASSERT(flat.fromFile(SOURCE_DIR "/shaders/flat.frag.glsl", Graphics::Shader::Type::Fragment), "Error loading fragment shader");
    flat.link();

    const auto square = [&]()
    {
        const std::vector<Math::Vec3f> points = {
            { -1.f,  1.f, 0.f },
            {  1.f, -1.f, 0.f },
            { -1.f, -1.f, 0.f },
            {  1.f,  1.f, 0.f }
        };

        const std::vector<Math::Vec2f> tex = {
            { 0.f, 1.f },
            { 1.f, 0.f },
            { 0.f, 0.f },
            { 1.f, 1.f }
        };

        const std::vector<uint32_t> indices = {
            0, 1, 2, 0, 3, 1
        };

        std::vector<Graphics::Vertex> vertices;
        for (uint32_t i = 0; i < 4; i++)
            vertices.push_back(Graphics::Vertex{ .color = Graphics::Color(255, 255, 255, 255), .position = points[i], .texCoords = tex[i] });

        Graphics::VertexArray v;
        v.upload(vertices);
        v.uploadIndices(indices);
        return v;
    }();

    Graphics::Texture texture;
    S2D_ASSERT(texture.fromFile(SOURCE_DIR "/textures/GRASS+.png"), "Error loading image");

    Graphics::DrawTexture surface;
    S2D_ASSERT(surface.create({ 1280, 720 }), "Error creating DrawTexture");

    auto vertices = []()
    {
        std::vector<Graphics::Vertex> vertices;

        const std::vector<Math::Vec3f> points = {
            { -0.5f,  0.5f, 0.f },
            {  0.5f, -0.5f, 0.f },
            { -0.5f, -0.5f, 0.f }
        };

        const std::vector<Math::Vec2f> tex = {
            { 0.f, 1.f },
            { 1.f, 0.f },
            { 0.f, 0.f }
        };

        for (uint32_t i = 0; i < points.size(); i++) 
            vertices.push_back(Graphics::Vertex{ .position = points[i], .color = Graphics::Color(255, 255, 255, 255), .texCoords = tex[i] });

        Graphics::VertexArray vao;
        vao.upload(vertices);
        return vao;
    }();

    while (window.isOpen())
    {
        Graphics::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Graphics::Event::Type::Close)
                window.close();
            if (event.type == Graphics::Event::Type::KeyPress)
            {
                logger->info("Key '{}' pressed", *event.keyPress.key);
                if (event.keyPress.key == Graphics::Keyboard::Key::Escape)
                    window.close();
            }
            if (event.type == Graphics::Event::Type::KeyRelease)
            {
                logger->info("Key '{}' released", *event.keyRelease.key);
            }
            if (event.type == Graphics::Event::Type::MousePress)
            {
                logger->info("Button '{}' pressed at ({}, {})", *event.mousePress.button, event.mousePress.position.x, event.mousePress.position.y);
            }
        }

        window.clear(Graphics::Color(255, 0, 0, 255));
        surface.clear(Graphics::Color(0, 255, 0, 255));

        Graphics::Context context;
        context.program = &program;
        context.textures.push_back(&texture);

        Math::Mat4f model(true);
        Math::Mat4f view(true);
        Math::Mat4f proj(true);
        
        const float aspectRatio = (float)window.getSize().x / (float)window.getSize().y;
        proj[1][1] = aspectRatio;

        program.setUniform("model", model);
        program.setUniform("view", view);
        program.setUniform("projection", proj);

        surface.draw(vertices, context);

        {
            Graphics::Context surface_context;
            surface_context.program = &flat;
            surface_context.textures.push_back(surface.texture());
            window.draw(square, surface_context);
        }

        window.display();
    }
}
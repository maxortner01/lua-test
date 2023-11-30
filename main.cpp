#include <Simple2D/Graphics.hpp>
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

    auto vertices = []()
    {
        std::vector<Graphics::Vertex> vertices;

        const std::vector<Math::Vec3f> points = {
            { -0.5f,  0.5f, 0.f },
            {  0.5f, -0.5f, 0.f },
            { -0.5f, -0.5f, 0.f }
        };

        for (const auto& p : points) vertices.push_back(Graphics::Vertex{ .position = p, .color = Graphics::Color(255, 0, 0, 255) });

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

        window.clear(Graphics::Color(255, 127, 0, 255));

        program.use();
        vertices.bind();
        vertices.draw();

        window.display();
    }
}
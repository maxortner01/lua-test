#include <Simple2D/Engine.hpp>

namespace S2D::Engine
{
    Application::Application(const sf::Vector2u& _size) :
        size(_size)
    {   }
}  

int main()
{
    using namespace S2D;

    auto* app = getApplication();
    Engine::Core core(*app);
    
    app->start(core);
    core.run();

    delete app;
}
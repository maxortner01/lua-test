#include <Simple2D/Engine.hpp>

namespace S2D::Engine
{
    Application::Application(const Math::Vec2u& _size) :
        size(_size)
    {   }
}  

int main()
{
    using namespace S2D;

    auto* app = getApplication();
    if (!app) return 1;

    Engine::Core core(*app);
    
    app->start(core);
    core.run();

    delete app;
    return 0;
}
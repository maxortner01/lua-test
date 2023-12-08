#include <Simple2D/Engine/Resources.hpp>
#include <Simple2D/Def.hpp>

#include <Simple2D/Graphics/Font.hpp>

#include <Simple2D/Log/Log.hpp>

#include <fstream>
#include <sstream>

namespace S2D::Engine
{

Resources::~Resources()
{   }

template<typename T>
Resources::Result<void>
Resources::loadResource(const std::string& name, const std::string& filename)
{
    const auto id = typeid(T).hash_code(); 
    if (!resources.count(id)) resources.insert(std::pair(id, ResourceMap()));
    auto& types = resources.at(id);
    if (types.count(name)) return { Error::AlreadyExists };
    auto* resource = new T();
    S2D_ASSERT(resource->fromFile(filename), "Resource failed to load");
    types.insert(std::pair(
        name,
        std::shared_ptr<void>(
            (void*)resource,
            [](void* ptr) { delete reinterpret_cast<T*>(ptr); }
        )
    ));
    return { };
}
template typename Resources::Result<void> Resources::loadResource<Graphics::Font>(const std::string&, const std::string&);
template typename Resources::Result<void> Resources::loadResource<Graphics::Texture>(const std::string&, const std::string&);
template typename Resources::Result<void> Resources::loadResource<Graphics::Image>(const std::string&, const std::string&);

template<>
Resources::Result<void>
Resources::loadResource<Graphics::Program>(const std::string& name, const std::string& filename, Graphics::Shader::Type type)
{
    const auto id = typeid(Graphics::Program).hash_code();
    if (!resources.count(id)) resources.insert(std::pair(id, ResourceMap()));
    auto& types = resources.at(id);
    Graphics::Program* res = nullptr;
    if (types.count(name)) res = (Graphics::Program*)types.at(name).get();
    else 
    {
        Log::Logger::instance("engine")->trace("Generating new shader");
        res = new Graphics::Program();
        types.insert(std::pair(
            name,
            std::shared_ptr<void>(
                (void*)res,
                [](void* ptr) { delete reinterpret_cast<Graphics::Program*>(ptr); }
            )
        ));
    }

    S2D_ASSERT(res, "Error loading/creating resource");

    const auto contents = [&]()
    {
        const auto path = std::filesystem::path(filename);
        std::ifstream file(path);
        S2D_ASSERT(file, "Error loading file");

        std::string line;
        std::stringstream ss;
        while (std::getline(file, line)) ss << line << "\n";

        return ss.str();
    }();

    S2D_ASSERT(res->fromString(contents, type), "Error loading shader from file");

    if (res->ready()) res->link();

    return { };
}

template<typename T>
Resources::Result<const T*>
Resources::getResource(const std::string& name) const
{
    const auto id = typeid(T).hash_code();
    S2D_ASSERT(resources.count(id), "Resource type does not exist");
    auto& types = resources.at(id);
    S2D_ASSERT(types.count(name), "Resource does not exist");
    return static_cast<const T*>(types.at(name).get());
}
template typename Resources::Result<const Graphics::Font*> Resources::getResource<Graphics::Font>(const std::string&) const;
template typename Resources::Result<const Graphics::Texture*> Resources::getResource<Graphics::Texture>(const std::string&) const;
template typename Resources::Result<const Graphics::Image*> Resources::getResource<Graphics::Image>(const std::string&) const;
template typename Resources::Result<const Graphics::Program*> Resources::getResource<Graphics::Program>(const std::string&) const;

template<typename T>
Resources::Result<T*>
Resources::getResource(const std::string& name)
{
    const auto id = typeid(T).hash_code();
    S2D_ASSERT(resources.count(id), "Resource type does not exist");
    auto& types = resources.at(id);
    S2D_ASSERT(types.count(name), "Resource does not exist");
    return static_cast<T*>(types.at(name).get());
}
template typename Resources::Result<Graphics::Font*> Resources::getResource<Graphics::Font>(const std::string&);
template typename Resources::Result<Graphics::Texture*> Resources::getResource<Graphics::Texture>(const std::string&);
template typename Resources::Result<Graphics::Image*> Resources::getResource<Graphics::Image>(const std::string&);
template typename Resources::Result<Graphics::Program*> Resources::getResource<Graphics::Program>(const std::string&);

}

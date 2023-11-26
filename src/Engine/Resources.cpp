#include <Simple2D/Engine/Resources.hpp>
#include <Simple2D/Def.hpp>

#include <SFML/Graphics.hpp>

namespace S2D::Engine
{

Resources::~Resources()
{
    for (auto& p : resources)
        for (auto& d : p.second)
            d.second.deleter(d.second.ptr);
    resources.clear();
}

template<typename T>
Resources::Result<void>
Resources::loadResource(const std::string& name, const std::string& filename)
{
    const auto id = typeid(T).hash_code(); 
    if (!resources.count(id)) resources.insert(std::pair(id, ResourceMap()));
    auto& types = resources.at(id);
    if (types.count(name)) return { Error::AlreadyExists };
    auto* resource = new T();
    S2D_ASSERT(resource->loadFromFile(filename), "Resource failed to load");
    types.insert(std::pair(name, DataPoint{
        .ptr     = (void*)resource,
        .deleter = [](void* ptr) { delete reinterpret_cast<T*>(ptr); }
    }));
    return { };
}
template typename Resources::Result<void> Resources::loadResource<sf::Font>(const std::string&, const std::string&);
template typename Resources::Result<void> Resources::loadResource<sf::Texture>(const std::string&, const std::string&);
template typename Resources::Result<void> Resources::loadResource<sf::Image>(const std::string&, const std::string&);

template<typename T>
Resources::Result<const T*>
Resources::getResource(const std::string& name) const
{
    const auto id = typeid(T).hash_code();
    S2D_ASSERT(resources.count(id), "Resource type does not exist");
    auto& types = resources.at(id);
    S2D_ASSERT(types.count(name), "Resource does not exist");
    return static_cast<const T*>(types.at(name).ptr);
}
template typename Resources::Result<const sf::Font*> Resources::getResource<sf::Font>(const std::string&) const;
template typename Resources::Result<const sf::Texture*> Resources::getResource<sf::Texture>(const std::string&) const;
template typename Resources::Result<const sf::Image*> Resources::getResource<sf::Image>(const std::string&) const;

}

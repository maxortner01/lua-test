#include <Simple2D/Engine/Resources.hpp>

#include <SFML/Graphics.hpp>

namespace S2D::Engine
{

template<typename T>
Resources::Result<void>
Resources::loadResource(const std::string& name, const std::string& filename)
{
    const auto id = typeid(T).hash_code();
    if (!resources.count(id)) resources.insert(std::pair(id, ResourceMap()));
    auto& types = resources.at(id);
    if (types.count(name)) return { Error::AlreadyExists };
    auto* resource = new T();
    assert(resource->loadFromFile(filename));
    types.insert(std::pair(name, std::shared_ptr<void>(
        (void*)resource,
        [](void* ptr) { delete reinterpret_cast<T*>(ptr); }
    )));
    return { };
}
template typename Resources::Result<void> Resources::loadResource<sf::Font>(const std::string&, const std::string&);
template typename Resources::Result<void> Resources::loadResource<sf::Texture>(const std::string&, const std::string&);

template<typename T>
Resources::Result<const T*>
Resources::getResource(const std::string& name) const
{
    const auto id = typeid(T).hash_code();
    assert(resources.count(id));
    auto& types = resources.at(id);
    assert(types.count(name));
    return static_cast<const T*>(types.at(name).get());
}
template typename Resources::Result<const sf::Font*> Resources::getResource<sf::Font>(const std::string&) const;
template typename Resources::Result<const sf::Texture*> Resources::getResource<sf::Texture>(const std::string&) const;

}

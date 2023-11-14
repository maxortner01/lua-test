#include <Simple2D/Engine/Resources.hpp>

#include <iostream>
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
    std::cout << "IN\n";
    const auto id = typeid(T).hash_code();
    std::cout << "yo\n";
    if (!resources.count(id)) resources.insert(std::pair(id, ResourceMap()));
    std::cout << "yo\n";
    auto& types = resources.at(id);
    if (types.count(name)) return { Error::AlreadyExists };
    auto* resource = new T();
    std::cout << resource << "\n";
    assert(resource->loadFromFile(filename));
    types.insert(std::pair(name, DataPoint{
        .ptr     = (void*)resource,
        .deleter = [](void* ptr) { delete reinterpret_cast<T*>(ptr); std::cout << "deleted\n"; }
    }));
    return { };
}
template typename Resources::Result<void> Resources::loadResource<sf::Font>(const std::string&, const std::string&);
template typename Resources::Result<void> Resources::loadResource<sf::Texture>(const std::string&, const std::string&);

template<typename T>
Resources::Result<const T*>
Resources::getResource(const std::string& name) const
{
    std::cout << "yo\n";
    const auto id = typeid(T).hash_code();
    assert(resources.count(id));
    auto& types = resources.at(id);
    assert(types.count(name));
    return static_cast<const T*>(types.at(name).ptr);
}
template typename Resources::Result<const sf::Font*> Resources::getResource<sf::Font>(const std::string&) const;
template typename Resources::Result<const sf::Texture*> Resources::getResource<sf::Texture>(const std::string&) const;

}

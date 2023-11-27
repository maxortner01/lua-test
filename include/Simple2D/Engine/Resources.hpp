#pragma once

#include "../Util.hpp"

#include <SFML/Graphics.hpp>
#include <unordered_map>

namespace S2D::Engine
{
    struct Resources 
    {
        enum class Error
        {
            AlreadyExists
        };

        struct DataPoint
        {
            void* ptr;
            std::function<void(void*)> deleter;
        };

        ~Resources();

        template<typename R>
        using Result = Util::Result<R, Util::Error<Error>>;

        template<typename T>
        Result<void>
        loadResource(const std::string& name, const std::string& filename);

        // Only works for T = sf::Shader
        template<typename T>
        Result<void>
        loadResource(const std::string& name, const std::string& filename, sf::Shader::Type type);

        template<typename T>
        Result<const T*>
        getResource(const std::string& name) const;

        template<typename T>
        Result<T*>
        getResource(const std::string& name);

    private:
        using ResourceMap = std::unordered_map<std::string, DataPoint>;
        std::unordered_map<std::size_t, ResourceMap> resources;
    };

}
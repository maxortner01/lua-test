#pragma once

#include "../Def.hpp"
#include "../Util.hpp"

namespace S2D::Graphics
{
    struct Shader
    {
        using Handle = uint32_t;

        enum class Type : uint8_t
        {
            Vertex, Fragment
        } type;
        Handle handle;
        std::optional<std::string> error;

        Shader(const Shader&) = delete;
        Shader(Shader&&)      = delete;

        bool good() const;

        Shader(const std::string& contents, Shader::Type type);
        ~Shader();

    };

    struct Program
    {
        using Handle = uint32_t;

        Program(const Program&) = delete;
        Program(Program&&)      = delete;

        Program()  = default;
        ~Program() = default;

        Util::Result<void> link();
        void use() const;

        [[nodiscard]]
        bool fromFile(const std::filesystem::path& filename, Shader::Type type);

        [[nodiscard]]
        bool fromString(const std::string& contents, Shader::Type type);

    private:
        Handle handle;
        std::unordered_map<Shader::Type, std::unique_ptr<Shader>> shaders;
    };
}
#include <Simple2D/Graphics/Shader.hpp>

#include <Simple2D/Log/Log.hpp>

#include <sstream>
#include <fstream>
#include <GL/glew.h>

namespace S2D::Graphics
{

GLenum get_type(Shader::Type type)
{
    switch (type)
    {
    case Shader::Type::Vertex:   return GL_VERTEX_SHADER;
    case Shader::Type::Fragment: return GL_FRAGMENT_SHADER;
    default: return GL_NONE;
    }
}

Shader::Shader(const std::string& contents, Shader::Type type) :
    handle(glCreateShader(get_type(type)))
{
    S2D_ASSERT(handle, "Error creating GL shader");

    const char* _contents = contents.c_str();
    glShaderSource(handle, 1, &_contents, nullptr);
    glCompileShader(handle);

    int success;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char buffer[512];
        glGetShaderInfoLog(handle, 512, nullptr, buffer);
        error = std::string(buffer);
        Log::Logger::instance("engine")->error("Error compiling {} shader: {}",
            ( type == Shader::Type::Vertex ? "Vertex" : "Fragment" ),
            error.value());
    }
}

Shader::~Shader()
{
    if (handle) glDeleteShader(handle);
    handle = 0;
}

bool Shader::good() const
{
    return !error.has_value();
}

Util::Result<void> Program::link()
{
    handle = glCreateProgram();
    S2D_ASSERT(handle, "Error creating GL program");
    for (const auto& shader : shaders)
        if (shader.second->good())
            glAttachShader(handle, shader.second->handle);
    glLinkProgram(handle);

    int success;
    glGetProgramiv(handle, GL_LINK_STATUS, &success);
    if (!success)
    {
        char buffer[512];
        glGetProgramInfoLog(handle, 512, nullptr, buffer);
        return { { success, std::string(buffer) } };
    }

    shaders.clear();

    return { };
}

bool Program::fromFile(const std::filesystem::path& filepath, Shader::Type type)
{
    std::ifstream file(filepath);
    S2D_ASSERT_ARGS(file, "Error opening file \"%s\" for %s shader compilation", 
        filepath.filename().c_str(), 
        ( type == Shader::Type::Vertex ? "Vertex" : "Fragment" ));

    std::string line;
    std::stringstream ss;
    while (std::getline(file, line)) ss << line << "\n";

    return fromString(ss.str(), type);
}

bool Program::fromString(const std::string& contents, Shader::Type type)
{
    S2D_ASSERT_ARGS(!shaders.count(type), "Program already contains shader type %s", 
        ( type == Shader::Type::Vertex ? "Vertex" : "Fragment" ));

    shaders.insert(std::pair(
        type, std::make_unique<Shader>(contents, type)
    ));

    return !shaders.at(type)->error.has_value();
}

}
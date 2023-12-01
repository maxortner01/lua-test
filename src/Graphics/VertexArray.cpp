#include <Simple2D/Graphics/VertexArray.hpp>
#include <Simple2D/Graphics/Surface.hpp>

#include <GL/glew.h>

namespace S2D::Graphics
{

Buffer::Buffer() :
    is_index(false)
{
    glGenBuffers(1, &handle);
    S2D_ASSERT(handle, "Error creating buffer");
}

Buffer::Buffer(Buffer&& buffer) :
    is_index(buffer.is_index),
    handle(buffer.handle)
{
    buffer.handle = 0;
}

Buffer::~Buffer()
{
    if (handle) glDeleteBuffers(1, &handle);
    handle = 0;
}

void Buffer::setData(const void* data, std::size_t byte_size, bool indices)
{
    if (indices) is_index = true;
    const auto type = ( indices ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER );
    glBindBuffer(type, handle);
    glBufferData(type, byte_size, data, GL_STATIC_DRAW);
}

VertexArray::VertexArray() :
    draw_type(DrawType::Triangles)
{
    glGenVertexArrays(1, &handle);
    S2D_ASSERT(handle, "Error creating VAO");
}

VertexArray::~VertexArray()
{
    if (handle) glDeleteVertexArrays(1, &handle);
    handle = 0;
}

void VertexArray::upload(const std::vector<Vertex>& vertices)
{
    bind();
    buffer.setData(vertices);
    
    if (!indices.has_value())
        count = vertices.size();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(0));
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(Math::Vec3f)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(Math::Vec3f) + sizeof(Color)));
    glEnableVertexAttribArray(2);    
}

void VertexArray::uploadIndices(const std::vector<uint32_t>& index_list)
{
    bind();
    indices.emplace();

    indices.value().setData(index_list, true);
    count = index_list.size();
}

void VertexArray::bind() const
{
    S2D_ASSERT(handle, "Can't bind VAO with invalid ID");
    glBindVertexArray(handle);
}

void VertexArray::setDrawType(DrawType type)
{
    draw_type = type;
}

void VertexArray::draw(Surface* window) const
{
    bind();

    const auto type = [&]()
    {
        switch (draw_type)
        {
        case DrawType::Triangles: return GL_TRIANGLES;
        case DrawType::Points:    return GL_POINTS;
        case DrawType::Lines:     return GL_LINES;
        default: return GL_NONE;
        }
    }();

    if (indices.has_value())
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.value().handle);
        glDrawElements(type, count, GL_UNSIGNED_INT, nullptr);
    }
    else
        glDrawArrays(type, 0, count);

    glBindVertexArray(0);
}

}
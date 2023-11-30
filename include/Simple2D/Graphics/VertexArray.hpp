#pragma once

#include "../Def.hpp"

#include "../Util/Color.hpp"

namespace S2D::Graphics
{

    struct Buffer
    {
        using Handle = uint32_t;

        Handle handle;
        bool   is_index;

        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&);

        Buffer();
        ~Buffer();

        void setData(const void* data, std::size_t byte_size, bool indices = false);

        template<typename T>
        void setData(const T* data, std::size_t count, bool indices = false);

        template<typename T>
        void setData(const std::vector<T>& data, bool indices = false);
    };

    struct Vertex
    {
        Math::Vec3f position;
        Color color;
        Math::Vec2f texCoords;
    };

    struct VertexArray
    {
        using Handle = uint32_t;

        VertexArray(const VertexArray&) = delete;

        VertexArray(bool needs_index = false);
        VertexArray(VertexArray&&) = default;
        
        ~VertexArray();

        void upload(const std::vector<Vertex>& vertices);
        void bind() const;
        void draw() const;

    private:
        std::optional<Buffer> indices;
        Buffer buffer;

        std::size_t count;
        Handle handle;
    };

    template<typename T>
    void Buffer::setData(const T* data, std::size_t count, bool indices)
    {
        setData(reinterpret_cast<const void*>(data), count * sizeof(T), indices);
    }

    template<typename T>
    void Buffer::setData(const std::vector<T>& data, bool indices)
    {
        setData(data.data(), data.size(), indices);
    }

}
#pragma once

#include "../Def.hpp"
#include "../Util/Color.hpp"

#include "Drawable.hpp"

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

    struct VertexArray : Drawable
    {
        using Handle = uint32_t;

        enum class DrawType
        {
            Triangles, Lines, Points
        };

        VertexArray(const VertexArray&) = delete;

        VertexArray();
        VertexArray(VertexArray&&) = default;
        
        ~VertexArray();

        void upload(const std::vector<Vertex>& vertices);
        void uploadIndices(const std::vector<uint32_t>& indices);

        void bind() const;
        void draw(Surface* window) const override;

        void setDrawType(DrawType type);
        DrawType getDrawType() const { return draw_type; }

        const std::size_t& vertexCount() const { return count; }

    private:
        std::optional<Buffer> indices;
        Buffer buffer;

        std::size_t count;
        Handle handle;
        DrawType draw_type;
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
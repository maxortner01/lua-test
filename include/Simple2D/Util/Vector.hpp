#pragma once

#include "../Def.hpp"

namespace S2D::Math
{
    template<typename _Type>
    struct Vec4
    {
        _Type x, y, z, w;
    };

    using Vec4f = Vec4<float>;
    using Vec4i = Vec4<int32_t>;
    using Vec4u = Vec4<uint32_t>;

    template<typename _Type>
    struct Vec3
    {
        _Type x, y, z;
    };

    using Vec3f = Vec3<float>;
    using Vec3i = Vec3<int32_t>;
    using Vec3u = Vec3<uint32_t>;

    template<typename _Type>
    struct Vec2
    {
        _Type x, y;
    };

    using Vec2f = Vec2<float>;
    using Vec2i = Vec2<int32_t>;
    using Vec2u = Vec2<uint32_t>;
}
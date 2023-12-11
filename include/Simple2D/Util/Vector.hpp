#pragma once

#include "../Def.hpp"

#include "Matrix.hpp"

namespace S2D::Math
{
    template<typename _Type>
    struct Vec4
    {
        _Type x, y, z, w;

        Mat<4, 1, _Type> matrix() const
        {
            Mat<4, 1, _Type> rows(false);
            rows.at(0, 0) = x;
            rows.at(1, 0) = y;
            rows.at(2, 0) = z;
            rows.at(3, 0) = w;
            return rows;
        }

        Vec4<_Type> transform(const Mat4f& matrix) const
        {
            Mat<4, 1, _Type> rows(false);
            rows.at(0, 0) = x;
            rows.at(1, 0) = y;
            rows.at(2, 0) = z;
            rows.at(3, 0) = w;

            const auto res = matrix * rows;
            return { res.at(0, 0), res.at(1, 0), res.at(2, 0), res.at(3, 0) };
        }
    };

    using Vec4f = Vec4<float>;
    using Vec4i = Vec4<int32_t>;
    using Vec4u = Vec4<uint32_t>;

    template<typename _Type>
    struct Vec3
    {
        _Type x, y, z;

        Vec3() : x(0), y(0), z(0)
        {   }

        Vec3(_Type _x, _Type _y, _Type _z) :
            x(_x), y(_y), z(_z)
        {   }

        _Type dot(const Vec3<_Type>& other) const
        {
            return x * other.x + y * other.y + z * other.z;
        }

        _Type length() const
        {
            return sqrt(x*x + y*y + z*z);
        }

        Vec3<_Type> normalized() const
        {
            const auto l = length();
            return { x / l, y / l, z / l };
        }

        template<typename _Other>
        operator Vec3<_Other>() const
        {
            return Vec3<_Other>(
                static_cast<_Other>(x),
                static_cast<_Other>(y),
                static_cast<_Other>(z)
            );
        }

        Vec3<_Type> operator+(const Vec3<_Type>& other) const
        {
            return Vec3<_Type>(
                x + other.x,
                y + other.y,
                z + other.z
            );
        }

        Vec3<_Type> operator/(const _Type& scalar) const
        {
            return {
                x / scalar, y / scalar, z / scalar
            };
        }

        Vec3<_Type> operator*(const _Type& scalar) const
        {
            return { x * scalar, y * scalar, z * scalar };
        }

        Vec3<_Type> operator-(const Vec3<_Type>& other) const
        {
            return {
                x - other.x, y - other.y, z - other.z
            };
        }

        Vec3<_Type> operator+=(const Vec3<_Type>& other)
        {
            x += other.x; y += other.y; z += other.z;
            return *this;
        }
    };

    using Vec3f = Vec3<float>;
    using Vec3i = Vec3<int32_t>;
    using Vec3u = Vec3<uint32_t>;

    template<typename _Type>
    struct Vec2
    {
        _Type x, y;

        Vec2() : x(0), y(0)
        {   }

        Vec2(_Type _x, _Type _y) :
            x(_x), y(_y)
        {   }

        _Type dot(const Vec2<_Type>& other) const
        {
            return x * other.x + y * other.y;
        }

        _Type length() const
        {
            return sqrt(x*x + y*y);
        }

        Vec2<_Type> normalized() const
        {
            const auto l = length();
            return { x / l, y / l };
        }

        template<typename _Other>
        operator Vec2<_Other>() const
        {
            return Vec2<_Other>(
                static_cast<_Other>(x),
                static_cast<_Other>(y)
            );
        }

        Vec2<_Type> operator+(const Vec2<_Type>& other) const
        {
            return Vec2<_Type>(
                x + other.x,
                y + other.y
            );
        }

        Vec2<_Type> operator/(const _Type& scalar) const
        {
            return {
                x / scalar, y / scalar
            };
        }

        Vec2<_Type> operator*(const _Type& scalar) const
        {
            return { x * scalar, y * scalar };
        }

        Vec2<_Type> operator-(const Vec2<_Type>& other) const
        {
            return {
                x - other.x, y - other.y
            };
        }

        Vec2<_Type> operator+=(const Vec2<_Type>& other)
        {
            x += other.x; y += other.y;
            return *this;
        }
    };

    using Vec2f = Vec2<float>;
    using Vec2i = Vec2<int32_t>;
    using Vec2u = Vec2<uint32_t>;
}

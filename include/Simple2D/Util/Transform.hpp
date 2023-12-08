#pragma once

#include "Vector.hpp"
#include "Matrix.hpp"
#include "Angle.hpp"

namespace S2D::Math
{
    struct Transform
    {
        Transform() :
            _scale({ 1.f, 1.f, 1.f })
        {   }

        ~Transform() = default;

        void translate(const Math::Vec3f& t)
        {
            translation += t;
        }

        void rotate(const Math::Vec3f& r)
        {
            rotation += r;
        }   

        void scale(const Math::Vec3f& s)
        {
            _scale = { _scale.x * s.x, _scale.y * s.y, _scale.z * s.z };
        }

        Mat4f matrix() const
        {
            Mat4f s;
            s[0][0] = _scale.x;
            s[1][1] = _scale.y;
            s[2][2] = _scale.z;

            Mat4f t;
            t[3][0] = translation.x;
            t[3][1] = translation.y;
            t[3][2] = translation.z;

            const auto rot_x = [&]() -> Mat4f
            {
                auto angle = Util::degrees(rotation.x);
                Mat4f mat;

                mat[1][1] = cos(angle.asRadians());
                mat[2][2] = cos(angle.asRadians());

                mat[1][2] = -1.f * sin(angle.asRadians());
                mat[2][1] = sin(angle.asRadians());

                return mat;
            }();

            const auto rot_y = [&]() -> Mat4f
            {
                auto angle = Util::degrees(rotation.y);
                Mat4f mat;

                mat[0][0] = cos(angle.asRadians());
                mat[2][2] = cos(angle.asRadians());

                mat[0][2] = sin(angle.asRadians());
                mat[2][0] = -1.f * sin(angle.asRadians());

                return mat;
            }();

            const auto rot_z = [&]() -> Mat4f
            {
                auto angle = Util::degrees(rotation.z);
                Mat4f mat;

                mat[0][0] = cos(angle.asRadians());
                mat[1][1] = cos(angle.asRadians());

                mat[0][1] = -1.f * sin(angle.asRadians());
                mat[1][0] = sin(angle.asRadians());

                return mat;
            }();
            
            return s * rot_x * rot_y * rot_z * t;
        }

        const Math::Vec3f& getRotation() const { return rotation; }
        const Math::Vec3f& getScale() const { return _scale; }
        const Math::Vec3f& getTranslation() const { return translation; }

    private:
        Math::Vec3f _scale;
        Math::Vec3f rotation;
        Math::Vec3f translation;
    };
}

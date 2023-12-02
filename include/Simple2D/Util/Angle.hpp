#pragma once

namespace S2D::Util
{
    struct Angle
    {
        Angle()  = default;
        ~Angle() = default;

        float asDegrees() const
        {
            return value / 0.0174533f;
        }

        float asRadians() const
        {
            return value;
        }

    private:
        friend constexpr Angle degrees(const float&);
        friend constexpr Angle radians(const float&);

        constexpr Angle(float rad) : value(rad) {  }

        float value;
    };

    constexpr Angle degrees(const float& deg)
    {
        return { deg * 0.0174533f }; 
    }

    constexpr Angle radians(const float& rad)
    {
        return { rad };
    }
}
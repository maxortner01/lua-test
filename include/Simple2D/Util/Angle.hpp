#pragma once

// SHould be S2D::Math
namespace S2D::Util
{
    struct Angle
    {
        static constexpr float CONVERSION = 3.14159f / 180.f;

        Angle()  = default;
        ~Angle() = default;

        float asDegrees() const
        {
            return value / Angle::CONVERSION;
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
        return { deg * Angle::CONVERSION }; 
    }

    constexpr Angle radians(const float& rad)
    {
        return { rad };
    }
}
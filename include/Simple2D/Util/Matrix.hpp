#pragma once

namespace S2D::Math
{
    template<typename T>
    struct Mat4
    {
        Mat4(bool identity = true);

        const T* mat() const;

        T& at(const uint8_t& row, const uint8_t& col);

        T* operator[](const uint8_t& row);

    private:
        T data[16];
    };

    template<typename T>
    Mat4<T>::Mat4(bool identity)
    {
        std::memset(data, 0, sizeof(T) * 16);
        if (identity)
            for (uint32_t i = 0; i < 4; i++)
                this->at(i, i) = 1.f;
    }

    template<typename T>
    const T* Mat4<T>::mat() const
    {
        return data;
    }

    template<typename T>
    T& Mat4<T>::at(const uint8_t& row, const uint8_t& col)
    {
        return data[row * 4 + col];
    }

    template<typename T>
    T* Mat4<T>::operator[](const uint8_t& row)
    {
        return &data[row * 4];
    }

    using Mat4f = Mat4<float>;
}
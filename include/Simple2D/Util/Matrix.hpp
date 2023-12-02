#pragma once

namespace S2D::Math
{
    template<int R, int C, typename T>
    struct Mat
    {
        Mat(bool identity = true)
        {
            std::memset(data, 0, sizeof(T) * 16);
            if (identity)
                for (uint32_t i = 0; i < R && i < C; i++)
                    this->at(i, i) = 1.f;
        }

        const T* mat() const
        {
            return data;
        }

        T& at(const uint8_t& row, const uint8_t& col)
        {
            return data[row * C + col];
        }

        const T& at(const uint8_t& row, const uint8_t& col) const
        {
            return data[row * C + col];
        }

        T* operator[](const uint8_t& row)
        {
            return &data[row * C];
        }

        // returns this * other
        template<int _C>
        Mat<R, _C, T> operator*(const Mat<C, _C, T>& other) const
        {
            Mat<R, _C, T> res(false);
            for (uint32_t i = 0; i < R; i++)
                for (uint32_t j = 0; j < _C; j++)
                    for (uint32_t k = 0; k < C; k++)
                        res[i][j] += at(i, k) * other.at(k, j);
            return res;
        }

        bool operator==(const Mat<R, C, T>& other) const
        {
            for (uint32_t r = 0; r < R; r++)
                for (uint32_t c = 0; c < C; c++)
                    if (at(r, c) != other.at(r, c))
                        return false;
            return true;
        }

        Mat<R, C, T> operator*=(const T& scalar)
        {
            for (uint32_t r = 0; r < R; r++)
                for (uint32_t c = 0; c < C; c++)
                    at(r, c) *= scalar;
            return *this;
        }

    private:
        T data[R * C];
    };

    using Mat4f = Mat<4, 4, float>;
}
#pragma once

namespace S2D::Math
{
    template<int R, int C, typename T>
    struct Mat
    {
        Mat(bool identity = true)
        {
            data = reinterpret_cast<T*>(std::calloc(16, sizeof(T)));
            if (identity)
                for (uint32_t i = 0; i < R && i < C; i++)
                    this->at(i, i) = 1.f;
        }

        Mat(Mat&& m) :
            data(m.data)
        {
            m.data = nullptr;
        }

        ~Mat()
        {
            if (data) std::free(data);
            data = nullptr;
        }

        const T* mat() const
        {
            return data;
        }

        T* mat()
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

        Mat<R, C, T> transpose() const
        {
            Mat<R, C, T> t(false);

            for (uint32_t r = 0; r < R; r++)
                for (uint32_t c = 0; c < C; c++)
                    t.at(c, r) = this->at(r, c);

            return t;   
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
        //T data[R * C];
        T* data;
    };

    using Mat4f = Mat<4, 4, float>;

    // Fun code from MESA
    // https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
    inline static Mat4f inverse(const Mat4f& mat)
    {
        Mat4f ret(false);

        ret.mat()[0] = mat.mat()[5]  * mat.mat()[10] * mat.mat()[15] - 
                mat.mat()[5]  * mat.mat()[11] * mat.mat()[14] - 
                mat.mat()[9]  * mat.mat()[6]  * mat.mat()[15] + 
                mat.mat()[9]  * mat.mat()[7]  * mat.mat()[14] +
                mat.mat()[13] * mat.mat()[6]  * mat.mat()[11] - 
                mat.mat()[13] * mat.mat()[7]  * mat.mat()[10];

        ret.mat()[4] = -mat.mat()[4]  * mat.mat()[10] * mat.mat()[15] + 
                mat.mat()[4]  * mat.mat()[11] * mat.mat()[14] + 
                mat.mat()[8]  * mat.mat()[6]  * mat.mat()[15] - 
                mat.mat()[8]  * mat.mat()[7]  * mat.mat()[14] - 
                mat.mat()[12] * mat.mat()[6]  * mat.mat()[11] + 
                mat.mat()[12] * mat.mat()[7]  * mat.mat()[10];

        ret.mat()[8] = mat.mat()[4]  * mat.mat()[9] * mat.mat()[15] - 
                mat.mat()[4]  * mat.mat()[11] * mat.mat()[13] - 
                mat.mat()[8]  * mat.mat()[5] * mat.mat()[15] + 
                mat.mat()[8]  * mat.mat()[7] * mat.mat()[13] + 
                mat.mat()[12] * mat.mat()[5] * mat.mat()[11] - 
                mat.mat()[12] * mat.mat()[7] * mat.mat()[9];

        ret.mat()[12] = -mat.mat()[4]  * mat.mat()[9] * mat.mat()[14] + 
                mat.mat()[4]  * mat.mat()[10] * mat.mat()[13] +
                mat.mat()[8]  * mat.mat()[5] * mat.mat()[14] - 
                mat.mat()[8]  * mat.mat()[6] * mat.mat()[13] - 
                mat.mat()[12] * mat.mat()[5] * mat.mat()[10] + 
                mat.mat()[12] * mat.mat()[6] * mat.mat()[9];

        ret.mat()[1] = -mat.mat()[1]  * mat.mat()[10] * mat.mat()[15] + 
                mat.mat()[1]  * mat.mat()[11] * mat.mat()[14] + 
                mat.mat()[9]  * mat.mat()[2] * mat.mat()[15] - 
                mat.mat()[9]  * mat.mat()[3] * mat.mat()[14] - 
                mat.mat()[13] * mat.mat()[2] * mat.mat()[11] + 
                mat.mat()[13] * mat.mat()[3] * mat.mat()[10];

        ret.mat()[5] = mat.mat()[0]  * mat.mat()[10] * mat.mat()[15] - 
                mat.mat()[0]  * mat.mat()[11] * mat.mat()[14] - 
                mat.mat()[8]  * mat.mat()[2] * mat.mat()[15] + 
                mat.mat()[8]  * mat.mat()[3] * mat.mat()[14] + 
                mat.mat()[12] * mat.mat()[2] * mat.mat()[11] - 
                mat.mat()[12] * mat.mat()[3] * mat.mat()[10];

        ret.mat()[9] = -mat.mat()[0]  * mat.mat()[9] * mat.mat()[15] + 
                mat.mat()[0]  * mat.mat()[11] * mat.mat()[13] + 
                mat.mat()[8]  * mat.mat()[1] * mat.mat()[15] - 
                mat.mat()[8]  * mat.mat()[3] * mat.mat()[13] - 
                mat.mat()[12] * mat.mat()[1] * mat.mat()[11] + 
                mat.mat()[12] * mat.mat()[3] * mat.mat()[9];

        ret.mat()[13] = mat.mat()[0]  * mat.mat()[9] * mat.mat()[14] - 
                mat.mat()[0]  * mat.mat()[10] * mat.mat()[13] - 
                mat.mat()[8]  * mat.mat()[1] * mat.mat()[14] + 
                mat.mat()[8]  * mat.mat()[2] * mat.mat()[13] + 
                mat.mat()[12] * mat.mat()[1] * mat.mat()[10] - 
                mat.mat()[12] * mat.mat()[2] * mat.mat()[9];

        ret.mat()[2] = mat.mat()[1]  * mat.mat()[6] * mat.mat()[15] - 
                mat.mat()[1]  * mat.mat()[7] * mat.mat()[14] - 
                mat.mat()[5]  * mat.mat()[2] * mat.mat()[15] + 
                mat.mat()[5]  * mat.mat()[3] * mat.mat()[14] + 
                mat.mat()[13] * mat.mat()[2] * mat.mat()[7] - 
                mat.mat()[13] * mat.mat()[3] * mat.mat()[6];

        ret.mat()[6] = -mat.mat()[0]  * mat.mat()[6] * mat.mat()[15] + 
                mat.mat()[0]  * mat.mat()[7] * mat.mat()[14] + 
                mat.mat()[4]  * mat.mat()[2] * mat.mat()[15] - 
                mat.mat()[4]  * mat.mat()[3] * mat.mat()[14] - 
                mat.mat()[12] * mat.mat()[2] * mat.mat()[7] + 
                mat.mat()[12] * mat.mat()[3] * mat.mat()[6];

        ret.mat()[10] = mat.mat()[0]  * mat.mat()[5] * mat.mat()[15] - 
                mat.mat()[0]  * mat.mat()[7] * mat.mat()[13] - 
                mat.mat()[4]  * mat.mat()[1] * mat.mat()[15] + 
                mat.mat()[4]  * mat.mat()[3] * mat.mat()[13] + 
                mat.mat()[12] * mat.mat()[1] * mat.mat()[7] - 
                mat.mat()[12] * mat.mat()[3] * mat.mat()[5];

        ret.mat()[14] = -mat.mat()[0]  * mat.mat()[5] * mat.mat()[14] + 
                mat.mat()[0]  * mat.mat()[6] * mat.mat()[13] + 
                mat.mat()[4]  * mat.mat()[1] * mat.mat()[14] - 
                mat.mat()[4]  * mat.mat()[2] * mat.mat()[13] - 
                mat.mat()[12] * mat.mat()[1] * mat.mat()[6] + 
                mat.mat()[12] * mat.mat()[2] * mat.mat()[5];

        ret.mat()[3] = -mat.mat()[1] * mat.mat()[6] * mat.mat()[11] + 
                mat.mat()[1] * mat.mat()[7] * mat.mat()[10] + 
                mat.mat()[5] * mat.mat()[2] * mat.mat()[11] - 
                mat.mat()[5] * mat.mat()[3] * mat.mat()[10] - 
                mat.mat()[9] * mat.mat()[2] * mat.mat()[7] + 
                mat.mat()[9] * mat.mat()[3] * mat.mat()[6];

        ret.mat()[7] = mat.mat()[0] * mat.mat()[6] * mat.mat()[11] - 
                mat.mat()[0] * mat.mat()[7] * mat.mat()[10] - 
                mat.mat()[4] * mat.mat()[2] * mat.mat()[11] + 
                mat.mat()[4] * mat.mat()[3] * mat.mat()[10] + 
                mat.mat()[8] * mat.mat()[2] * mat.mat()[7] - 
                mat.mat()[8] * mat.mat()[3] * mat.mat()[6];

        ret.mat()[11] = -mat.mat()[0] * mat.mat()[5] * mat.mat()[11] + 
                mat.mat()[0] * mat.mat()[7] * mat.mat()[9] + 
                mat.mat()[4] * mat.mat()[1] * mat.mat()[11] - 
                mat.mat()[4] * mat.mat()[3] * mat.mat()[9] - 
                mat.mat()[8] * mat.mat()[1] * mat.mat()[7] + 
                mat.mat()[8] * mat.mat()[3] * mat.mat()[5];

        ret.mat()[15] = mat.mat()[0] * mat.mat()[5] * mat.mat()[10] - 
                mat.mat()[0] * mat.mat()[6] * mat.mat()[9] - 
                mat.mat()[4] * mat.mat()[1] * mat.mat()[10] + 
                mat.mat()[4] * mat.mat()[2] * mat.mat()[9] + 
                mat.mat()[8] * mat.mat()[1] * mat.mat()[6] - 
                mat.mat()[8] * mat.mat()[2] * mat.mat()[5];

        auto det = mat.mat()[0] * ret.mat()[0] + mat.mat()[1] * ret.mat()[4] + mat.mat()[2] * ret.mat()[8] + mat.mat()[3] * ret.mat()[12];
        S2D_ASSERT(det, "Determinant vanishes!");

        det = 1.0 / det;

        Mat4f fin(false);

        for (uint32_t i = 0; i < 16; i++)
            fin.mat()[i] = ret.mat()[i] * det;

        return fin;
    }
}

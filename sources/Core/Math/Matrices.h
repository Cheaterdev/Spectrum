#pragma once

// General row major matrix data
template<typename int _N, typename int _M, typename data_type>
class matrix_data_t
{
    public:
        static const int N = _N;
        static const int M = _M;
        typedef data_type Format;
        typedef Vector<vector_data_t<Format, N>> RowFormat;
        typedef Vector<vector_data_t<Format, M>> ColumnFormat;

		union
		{
			struct
			{
				RowFormat rows[M];
			};
		
			struct
			{
				Format elems[N*M];
			};
		};

        matrix_data_t() {};
};

template<typename matrix_type>
class matrix : public matrix_type
{
    using matrix_type::rows;
    using matrix_type::N;
	using matrix_type::M;   using matrix_type::Format;

/*	template<class T2>
	int set_internal(int i, T2 d,
		typename std::enable_if<std::is_compound<T2>::value>::type* = 0)
	{
		for (int t = 0; t < std::min(T::N, T2::N); t++)
			T::values[i++] = static_cast<T::Format>(d.values[t]);

		return i;
	}*/

	template<class T2>
	int set_internal(int i, T2 d,
		typename std::enable_if < !std::is_compound<T2>::value >::type* = 0)
	{
		matrix_type::elems[i++] = static_cast<typename matrix_type::Format>(d);
		return i;
	}

	template<class D,class C, class ...Args>
	int set_internal(int i, D d, C c,Args ...args)
	{
		return set_internal(set_internal(i, d),c, args...);
	}

	int set_internal(int)
	{
		return 0;


	}
    public:

        matrix() {};
        matrix(const matrix&) = default;
        matrix& operator=(const matrix&) = default;



		template<class ...Args>
		matrix(Args ...args)
		{
			set_internal(0, args...);
		}



        matrix& ZeroMatrix()
        {
            ZeroMemory(rows, N * M * sizeof(Format));
            return *this;
        }
        typename matrix_type::Format* raw()
        {
            return &rows[0][0];
        }
        typename matrix_type::RowFormat& operator[](unsigned int i) { return rows[i]; }
        typename const matrix_type::RowFormat& operator[](unsigned int i) const { return rows[i]; }

        typename matrix_type::RowFormat GetRow(unsigned int i) const { return rows[i]; }
        typename matrix_type::ColumnFormat GetColumn(unsigned int j) const
        {
           typename matrix_type::ColumnFormat v;

            for (int i = 0; i < N; ++i)
                v[i] = rows[i][j];

            return v;
        }
        typename const matrix_type::Format& operator()(unsigned int i, unsigned int j) const
        {
            return rows[i][j];
        }

        typename matrix_type::Format& operator()(unsigned int i, unsigned int j)
        {
            return rows[i][j];
        }

        // basic math operations
        FORCEINLINE matrix operator+(const matrix& m) const
        {
            matrix result;

            for (int i = 0; i < N; ++i)
                result[i] = rows[i] + m[i];

            return result;
        }
        FORCEINLINE matrix operator-(const matrix& m) const
        {
            matrix result;

            for (int i = 0; i < N; ++i)
                result[i] = rows[i] - m[i];

            return result;
        }
        FORCEINLINE matrix& operator-()
        {
            for (int i = 0; i < N; ++i)
                rows[i] = -rows[i];

            return *this;
        }
        FORCEINLINE matrix& operator+=(const matrix& m)
        {
            for (int i = 0; i < N; ++i)
                rows[i] += m[i];

            return *this;
        }
        FORCEINLINE matrix& operator-=(const matrix& m)
        {
            for (int i = 0; i < N; ++i)
                rows[i] -= m[i];

            return *this;
        }

        FORCEINLINE matrix operator*(const matrix& m) const
        {
            matrix r; r.ZeroMatrix();

            for (int i = 0; i < N; i++)
                for (int j = 0; j < N; j++)
                    for (int k = 0; k < N; k++)
                        r(i, j) += (*this)(i, k) * m(k, j);

            return r;
        }

        FORCEINLINE matrix& operator*=(const matrix& m)
        {
            matrix r; r.ZeroMatrix();

            for (int i = 0; i < N; i++)
                for (int j = 0; j < N; j++)
                    for (int k = 0; k < N; k++)
                        r(i, j) += (*this)(i, k) * m(k, j);

            *this = r;
            return *this;
        }

        template<class _t = matrix_type>
        static 	matrix translation(vec3 dpos, typename std::enable_if<_t::N == 4>::type* = 0)
        {
            matrix r;
            r.a11 = r.a22 = r.a33 = r.a44 = 1.0f;
            r.a12 = r.a13 = r.a14 = 0.0f;
            r.a21 = r.a23 = r.a24 = 0.0f;
            r.a31 = r.a32 = r.a34 = 0.0f;
            r.a41 = dpos.x;
            r.a42 = dpos.y;
            r.a43 = dpos.z;
            return r;
        }

        // Transformation matrices

        template<class _t = matrix_type>
        static 	matrix rotationX(float angle, typename std::enable_if<_t::N == 4>::type* = 0)
        {
            matrix r;
            float cosA = cos(angle);
            float sinA = sin(angle);
            r.a11 = r.a44 = 1.0f;
            r.a12 = r.a13 = r.a14 = 0.0f;
            r.a21 = r.a24 = 0.0f;
            r.a31 = r.a34 = 0.0f;
            r.a41 = r.a42 = r.a43 = 0.0f;
            r.a22 = r.a33 = cosA;
            r.a23 = sinA;
            r.a32 = -sinA;
            return r;
        }

        template<class _t = matrix_type>
        static 	matrix rotationY(float angle, typename std::enable_if<_t::N == 4>::type* = 0)
        {
            matrix r;
            float cosA = cos(angle);
            float sinA = sin(angle);
            r.a22 = r.a44 = 1.0f;
            r.a12 = r.a14 = 0.0f;
            r.a21 = r.a23 = r.a24 = 0.0f;
            r.a32 = r.a34 = 0.0f;
            r.a41 = r.a42 = r.a43 = 0.0f;
            r.a11 = r.a33 = cosA;
            r.a13 = -sinA;
            r.a31 = sinA;
            return r;
        }

        template<class _t = matrix_type>
        static 	matrix rotationZ(float angle, typename std::enable_if<_t::N == 4>::type* = 0)
        {
            matrix r;
            float cosA = cos(angle);
            float sinA = sin(angle);
            r.a33 = r.a44 = 1.0f;
            r.a13 = r.a14 = 0.0f;
            r.a23 = r.a24 = 0.0f;
            r.a31 = r.a32 = r.a34 = 0.0f;
            r.a41 = r.a42 = r.a43 = 0.0f;
            r.a11 = r.a22 = cosA;
            r.a12 = sinA;
            r.a21 = -sinA;
            return r;
        }


};

template<typename matrix_type>
FORCEINLINE matrix<matrix_type> operator*(float n, const matrix<matrix_type>& m)
{
    matrix<matrix_type> result;

    for (int i = 0; i < matrix_type::N; ++i)
        result[i] = n * m[i];

    return result;
}

template<typename matrix_type>
FORCEINLINE matrix<matrix_type> operator*(const matrix<matrix_type>& m, float n)
{
    matrix<matrix_type> result;

    for (int i = 0; i < matrix_type::N; ++i)
        result[i] = n * m[i];

    return result;
}

// Specific row major matrix 4x4 data
class quat;


template<typename data_type>
class matrix_data_4x4_t
{
    public:
        static const int N = 4;
        static const int M = 4;
        typedef data_type Format;
        typedef Vector<vec4_t<Format>> RowFormat;
        typedef Vector<vec4_t<Format>> ColumnFormat;
        matrix_data_4x4_t() {};

        union
        {
            struct
            {
                RowFormat rows[M];
            };
            struct
            {
                Format a11, a12, a13, a14,
                       a21, a22, a23, a24,
                       a31, a32, a33, a34,
                       a41, a42, a43, a44;
            };
			struct
			{
				Format elems[N*M];
			};
        };

        matrix_data_4x4_t& identity()
        {
            a11 = a22 = a33 = a44 = 1;
            a12 = a13 = a14 = 0;
            a21 = a23 = a24 = 0;
            a31 = a32 = a34 = 0;
            a41 = a42 = a43 = 0;
            return *this;
        }
        Format det() const
        {
            Format d = 0;
            d += a11 * (a22 * a33 * a44 + a32 * a43 * a24 + a23 * a34 * a42 - (a24 * a33 * a42 + a32 * a23 * a44 + a43 * a34 * a22));
            d -= a12 * (a21 * a33 * a44 + a31 * a43 * a24 + a23 * a34 * a41 - (a24 * a33 * a41 + a31 * a23 * a44 + a43 * a34 * a21));
            d += a13 * (a21 * a32 * a44 + a31 * a42 * a24 + a22 * a34 * a41 - (a24 * a32 * a41 + a31 * a22 * a44 + a42 * a34 * a21));
            d -= a14 * (a21 * a32 * a43 + a31 * a42 * a23 + a22 * a33 * a41 - (a23 * a32 * a41 + a31 * a22 * a43 + a42 * a33 * a21));
            return d;
        }

        bool inverse(/*float threshold = eps4*/)
        {
            Format d = det();
            //  if (fabs((float)d) < threshold)
            //    return false;
            d = 1 / d;
            // Cached values
            auto a22_a33 = a22 * a33;
            matrix_data_4x4_t<data_type> m;
            m.a11 = a22_a33 * a44 + a32 * a43 * a24 + a23 * a34 * a42 - a24 * a33 * a42 - a32 * a23 * a44 - a43 * a34 * a22;
            m.a21 = a24 * a33 * a41 + a31 * a23 * a44 + a43 * a34 * a21 - a21 * a33 * a44 - a31 * a43 * a24 - a23 * a34 * a41;
            m.a31 = a21 * a32 * a44 + a31 * a42 * a24 + a22 * a34 * a41 - a24 * a32 * a41 - a31 * a22 * a44 - a42 * a34 * a21;
            m.a41 = a23 * a32 * a41 + a31 * a22 * a43 + a42 * a33 * a21 - a21 * a32 * a43 - a31 * a42 * a23 - a22_a33 * a41;
            m.a12 = a14 * a33 * a42 + a32 * a13 * a44 + a43 * a34 * a12 - a12 * a33 * a44 - a32 * a43 * a14 - a13 * a34 * a42;
            m.a22 = a11 * a33 * a44 + a31 * a43 * a14 + a13 * a34 * a41 - a14 * a33 * a41 - a31 * a13 * a44 - a43 * a34 * a11;
            m.a32 = a14 * a32 * a41 + a31 * a12 * a44 + a42 * a34 * a11 - a11 * a32 * a44 - a31 * a42 * a14 - a12 * a34 * a41;
            m.a42 = a11 * a32 * a43 + a31 * a42 * a13 + a12 * a33 * a41 - a13 * a32 * a41 - a31 * a12 * a43 - a42 * a33 * a11;
            m.a13 = a12 * a23 * a44 + a22 * a43 * a14 + a13 * a24 * a42 - a14 * a23 * a42 - a22 * a13 * a44 - a43 * a24 * a12;
            m.a23 = a14 * a23 * a41 + a21 * a13 * a44 + a43 * a24 * a11 - a11 * a23 * a44 - a21 * a43 * a14 - a13 * a24 * a41;
            m.a33 = a11 * a22 * a44 + a21 * a42 * a14 + a12 * a24 * a41 - a14 * a22 * a41 - a21 * a12 * a44 - a42 * a24 * a11;
            m.a43 = a13 * a22 * a41 + a21 * a12 * a43 + a42 * a23 * a11 - a11 * a22 * a43 - a21 * a42 * a13 - a12 * a23 * a41;
            m.a14 = a14 * a23 * a32 + a22 * a13 * a34 + a33 * a24 * a12 - a12 * a23 * a34 - a22_a33 * a14 - a13 * a24 * a32;
            m.a24 = a11 * a23 * a34 + a21 * a33 * a14 + a13 * a24 * a31 - a14 * a23 * a31 - a21 * a13 * a34 - a33 * a24 * a11;
            m.a34 = a14 * a22 * a31 + a21 * a12 * a34 + a32 * a24 * a11 - a11 * a22 * a34 - a21 * a32 * a14 - a12 * a24 * a31;
            m.a44 = a11 * a22_a33 + a21 * a32 * a13 + a12 * a23 * a31 - a13 * a22 * a31 - a21 * a12 * a33 - a32 * a23 * a11;
            Format* data = (Format*)&m;

            for (int i = 0; i < 16; ++i)
                data[i] *= d;

            *this = m;
            return true;
        }
        matrix_data_4x4_t& transpose()
        {
            // row 1
            swap(a12, a21);
            swap(a13, a31);
            swap(a14, a41);
            // row 2
            swap(a23, a32);
            swap(a24, a42);
            // row 3
            swap(a34, a43);
            return *this;
        }


        matrix_data_4x4_t& rotationAxis(vec3 dir, float angle)
        {
            quat q(dir, angle);
          //  q.toMatrix(*this);
            return *this;
        }

        matrix_data_4x4_t& scaling(float scale)
        {
            return scaling(scale, scale, scale);
        }
        matrix_data_4x4_t& scaling(vec3 scale)
        {
            return scaling(scale.x, scale.y, scale.z);
        }
        matrix_data_4x4_t& scaling(float scaleX, float scaleY, float scaleZ)
        {
            a11 = scaleX;
            a22 = scaleY;
            a33 = scaleZ;
            a44 = 1.0f;
            a12 = a13 = a14 = 0.0f;
            a21 = a23 = a24 = 0.0f;
            a31 = a32 = a34 = 0.0f;
            a41 = a42 = a43 = 0.0f;
            return *this;
        }
        matrix_data_4x4_t& look_at(const vec3& eye, const vec3& at, const vec3& up)
        {
            vec3 zaxis = (at - eye).normalize();
            vec3 xaxis = vec3::cross(up, zaxis).normalize();
            vec3 yaxis = vec3::cross(zaxis, xaxis);
            a11 = xaxis.x; a12 = yaxis.x; a13 = zaxis.x; a14 = 0;
            a21 = xaxis.y; a22 = yaxis.y; a23 = zaxis.y; a24 = 0;
            a31 = xaxis.z; a32 = yaxis.z; a33 = zaxis.z; a34 = 0;
            a41 = -vec3::dot(xaxis, eye);
            a42 = -vec3::dot(yaxis, eye);
            a43 = -vec3::dot(zaxis, eye);
            a44 = 1;
            return *this;
        }
        matrix_data_4x4_t& perspective(float fovy, float aspect, float zn, float zf)
        {
            float yScale = 1 / tan(0.5f * fovy);
            float xScale = yScale / aspect;
            a11 = xScale; a12 = 0; a13 = 0; a14 = 0;
            a21 = 0; a22 = yScale; a23 = 0; a24 = 0;
            a31 = 0; a32 = 0; a33 = zf / (zf - zn); a34 = 1;
            a41 = 0; a42 = 0; a43 = -zn * a33; a44 = 0;
            return *this;
        }
        matrix_data_4x4_t& orthographic(float l, float r, float t, float b, float zn, float zf)
        {
            /*
            2/(r-l)      0            0           0
            0            2/(t-b)      0           0
            0            0            1/(zf-zn)   0
            (l+r)/(l-r)  (t+b)/(b-t)  zn/(zn-zf)  1
            */
            a11 = 2 / (r - l); a12 = 0; a13 = 0; a14 = 0;
            a21 = 0; a22 = -2 / (t - b); a23 = 0; a24 = 0;
            a31 = 0; a32 = 0; a33 = -1 / (zn - zf); a34 = 0;
            a41 = (l + r) / (l - r); a42 = -(t + b) / (b - t); a43 = zn / (zn - zf); a44 = 1;
            return *this;
        }
        matrix_data_4x4_t& orthogonalize()
        {
            // Gram-Schmidt
            vec3 a(a11, a21, a31),
                 b(a12, a22, a32),
                 c(a13, a23, a33);
            float l;
            // a
            a.norm();
            // b
            l = vec3::dot(a, b);
            b -= l * a;
            b.norm();
            // c
            l = vec3::dot(a, c);
            c -= l * a;
            l = vec3::dot(b, c);
            c -= l * b;
            c.norm();
            a11 = a.x; a21 = a.y; a31 = a.z;
            a12 = b.x; a22 = b.y; a32 = b.z;
            a13 = c.x; a23 = c.y; a33 = c.z;
            return *this;
        }

        matrix_data_4x4_t& skew(vec3& v)
        {
            a11 = 0;	a12 = -v.z;	a13 = v.y;	a14 = 0;
            a21 = v.z;	a22 = 0;	a23 = -v.x;	a24 = 0;
            a31 = -v.y;	a32 = v.x;	a33 = 0;	a34 = 0;
            a41 = 0;	a42 = 0;	a43 = 0;	a44 = 1;
            return *this;
        }
        matrix_data_4x4_t& skew(vec4& v)
        {
            a11 = 0;	a12 = -v.z;	a13 = v.y;	a14 = 0;
            a21 = v.z;	a22 = 0;	a23 = -v.x;	a24 = 0;
            a31 = -v.y;	a32 = v.x;	a33 = 0;	a34 = 0;
            a41 = 0;	a42 = 0;	a43 = 0;	a44 = 1;
            return *this;
        }

        matrix_data_4x4_t& skew(quat& q)
        {
            a11 = -q.x;	a12 = -q.y;	a13 = -q.z;	a14 = 0;
            a21 = q.w;	a22 = q.z;	a23 = -q.y;	a24 = 0;
            a31 = -q.z;	a32 = q.w;	a33 = q.x;	a34 = 0;
            a41 = q.y;	a42 = -q.x;	a43 = q.w;	a44 = 1;
            return *this;
        }
		bool operator!=(const matrix_data_4x4_t& m)
		{
			for (int i = 0; i < 16; i++)
				if (elems[i] != m.elems[i])
					return true;

			return false;
		}
        // matrix multiplication
        /*matrix_data_4x4_t operator*(const matrix_data_4x4_t& m) const;
        matrix_data_4x4_t& operator*=(const matrix_data_4x4_t& m);*/
        /*
        	mat<T> operator*(const mat<T>& m) const
        	{
        		mat<T> r;

        		for (int i = 0; i < T::N; i++)
        		for (int j = 0; j < T::N; j++)
        		for (int k = 0; k < T::N; k++)
        			r(i, j) += (*this)(i, k) * m(k, j);
        		return r;
        	}
        	*/
};

using mat4x4 = matrix<matrix_data_4x4_t<float>>;
using mat4x3 = matrix<matrix_data_t<3,4, float>>;


template<typename VectorFormatIn, typename MatrixFormat>
Vector<vector_data_t<typename MatrixFormat::Format, MatrixFormat::M>>operator*(const Vector<VectorFormatIn>& v, const  matrix<MatrixFormat>& m)
{
    static_assert(VectorFormatIn::N == MatrixFormat::M, "can't multiply vector and matrix, bad format");
    const int N = MatrixFormat::N;
    const int M = MatrixFormat::M;
    typedef MatrixFormat::Format Format;
    Vector<vector_data_t<Format, M>> result;

    for (int i = 0; i < M; ++i)
    {
        result[i] = 0;

        for (int j = 0; j < N; ++j)
            result[i] += v[j] * m[j][i];
    }

    return result;
}

template<typename T, typename T2>
typename Vector<vec3_t<T>> operator*(Vector<vec3_t<T>> v, matrix<matrix_data_4x4_t<T2>> m)
{
    Vector<vec4_t<T>> data(v, 1);
    data = data * m;
    return Vector<vec3_t<T>>(data) / data[3];
}
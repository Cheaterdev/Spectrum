module;
#include <type_traits>

export module Matrices;


import Quaternion;
import Vectors;
import Constants;

#include "Serialization/serialization.h"

export {

	template<typename matrix_type>
	class matrix : public matrix_type
	{
		using Format = typename matrix_type::Format;
		using matrix_type::rows;
		using matrix_type::N;
		using matrix_type::M;

		template<class T2>
		int set_internal(int i, T2 d) requires(std::is_compound_v<T2>)
		{
			matrix_type::elems[i++] = static_cast<typename matrix_type::Format>(d);
			return i;
		}

		template<class D, class C, class ...Args>
		int set_internal(int i, D d, C c, Args ...args)
		{
			return set_internal(set_internal(i, d), c, args...);
		}

		int set_internal(int)
		{
			return 0;
		}
	public:

		using type = matrix<matrix_type>;

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
			for (int i = 0; i < matrix_type::elems.size(); i++)
				matrix_type::elems[i] = 0;

			return *this;
		}

		Format* raw()
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
		const Format& operator()(unsigned int i, unsigned int j) const
		{
			return rows[i][j];
		}

		Format& operator()(unsigned int i, unsigned int j)
		{
			return rows[i][j];
		}

		// basic math operations
		inline matrix operator+(const matrix& m) const
		{
			matrix result;

			for (int i = 0; i < N; ++i)
				result[i] = rows[i] + m[i];

			return result;
		}
		inline matrix operator-(const matrix& m) const
		{
			matrix result;

			for (int i = 0; i < N; ++i)
				result[i] = rows[i] - m[i];

			return result;
		}
		inline matrix& operator-()
		{
			for (int i = 0; i < N; ++i)
				rows[i] = -rows[i];

			return *this;
		}
		inline matrix& operator+=(const matrix& m)
		{
			for (int i = 0; i < N; ++i)
				rows[i] += m[i];

			return *this;
		}
		inline matrix& operator-=(const matrix& m)
		{
			for (int i = 0; i < N; ++i)
				rows[i] -= m[i];

			return *this;
		}

		inline matrix operator*(const matrix& m) const
		{
			matrix r; r.ZeroMatrix();

			for (int i = 0; i < N; i++)
				for (int j = 0; j < N; j++)
					for (int k = 0; k < N; k++)
						r(i, j) += (*this)(i, k) * m(k, j);

			return r;
		}

		inline matrix& operator*=(const matrix& m)
		{
			matrix r; r.ZeroMatrix();

			for (int i = 0; i < N; i++)
				for (int j = 0; j < N; j++)
					for (int k = 0; k < N; k++)
						r(i, j) += (*this)(i, k) * m(k, j);

			*this = r;
			return *this;
		}

		bool operator!=(const matrix& m)
		{
			for (int i = 0; i < 16; i++)
				if (matrix_type::elems[i] != m.elems[i])
					return true;

			return false;
		}

#include "matrix4_functions.h"


	private:
		SERIALIZE()
		{
			ar& NP("elems", matrix_type::elems);
		}

	};

	template<typename matrix_type>
	inline matrix<matrix_type> operator*(float n, const matrix<matrix_type>& m)
	{
		matrix<matrix_type> result;

		for (int i = 0; i < matrix_type::N; ++i)
			result[i] = n * m[i];

		return result;
	}

	template<typename matrix_type>
	inline matrix<matrix_type> operator*(const matrix<matrix_type>& m, float n)
	{
		matrix<matrix_type> result;

		for (int i = 0; i < matrix_type::N; ++i)
			result[i] = n * m[i];

		return result;
	}

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
				Vector<vector_data_t<Format, N, false>> rows[M];
			};

			struct
			{
				std::array<Format, N* M> elems;
			};
		};

		matrix_data_t() {};
	};

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
				std::array<Format, N* M> elems;
			};
		};
	};


	using mat4x4 = matrix<matrix_data_4x4_t<float>>;
	using mat4x3 = matrix<matrix_data_t<3, 4, float>>;
	using float4x4 = mat4x4;

	template<typename VectorFormatIn, typename MatrixFormat>
	auto operator*(const Vector<VectorFormatIn>& v, const  matrix<MatrixFormat>& m) requires(VectorFormatIn::N == MatrixFormat::M)
	{
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
		return Vector<vec3_t<T>>(data.xyz) / data[3];
	}

}
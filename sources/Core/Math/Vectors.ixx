
#include "Constants.h"
#include "LOg/simple_archive.h"

#include <numeric>
#include <array>

export module Vectors;

export import Constants;
export {

	// Basic vector
	template<typename T>
	class Vector : public T
	{

		template<int i, class D, class ...Args>
		void set_internal(D d, Args ...args)
		{
			if constexpr (std::is_compound_v<D>) {
				constexpr int size = std::min(T::N, D::N);
				for (int t = 0; t < size; t++)
					T::values[i + t] = static_cast<typename T::Format>(d.values[t]);
				set_internal<i + size>(args...);
			}
			else {
				T::values[i] = static_cast<typename T::Format>(d);
				set_internal<i + 1>(args...);
			}

		}
		template<int i>
		void set_internal()
		{
		}



	public:
		using T::values;
		using T::Format;
		using T::N;
		using raw = typename T;
		template<typename = typename std::enable_if_t<T::GENERATE_CONSTRUCTOR>>
		Vector()
		{
			memset(T::values.data(), 0, sizeof(T::Format) * T::N);
		}

		template< typename = typename std::enable_if_t<T::GENERATE_CONSTRUCTOR>, class ...Args>
		Vector(Args ...args)
		{
			set(args...);
		}

		template<class ...Args>
		void set(Args ...args)
		{
			set_internal<0>(args...);
		}

		typename T::Format* data()
		{
			return this->values.data();
		}
		typename T::Format& operator[](unsigned int i) { return this->values[i]; }
		typename const T::Format& operator[](unsigned int i) const { return this->values[i]; }

		template<class T2, typename = typename std::enable_if_t<N == T2::N>>
		Vector& operator=(const Vector<T2>& v)
		{
			for (int i = 0; i < T::N; ++i)
				values[i] = static_cast<typename T::Format>(v[i]);
			return *this;
		}

		inline Vector operator+(const Vector& v) const
		{
			Vector result;

			for (int i = 0; i < T::N; ++i)
				result[i] = values[i] + v[i];

			return result;
		}

		template<class T2>
		inline Vector operator-(const Vector<T2>& v) const
		{
			Vector result(*this);

			for (int i = 0; i < T::N; ++i)
				result[i] = values[i] - v[i];

			return result;
		}

		inline Vector operator-() const
		{
			Vector<T> result;

			for (int i = 0; i < T::N; ++i)
				result[i] = -values[i];

			return result;
		}

		inline Vector& operator+=(const Vector& v)
		{
			for (int i = 0; i < T::N; ++i)
				values[i] += v[i];

			return *this;
		}

		inline Vector& operator-=(const Vector& v)
		{
			for (int i = 0; i < T::N; ++i)
				values[i] -= v[i];

			return *this;
		}

		inline Vector& operator*=(typename const T::Format& n)
		{
			for (int i = 0; i < T::N; ++i)
				values[i] *= n;

			return *this;
		}

		inline Vector& operator/=(typename const T::Format& n)
		{
			for (int i = 0; i < T::N; ++i)
				values[i] /= n;

			return *this;
		}

		inline typename T::Format length_squared() const
		{
			return dot(*this, *this);
		}

		inline decltype(std::sqrt(typename T::Format())) length() const
		{
			return std::sqrt((double)length_squared());
		}

		Vector<T>& normalize(float eps = eps12)
		{
			auto d = length_squared();

			if (d > eps)
				(*this) /= std::sqrt((double)d);

			return *this;
		}


		Vector<T> normalized(float eps = eps12)
		{
			Vector res(*this);
			return res.normalize();
		}

		typename T::Format max_element()
		{
			return *std::max_element(values.begin(), values.end());
		}

		typename T::Format sum()
		{
			return std::accumulate(values.begin(), values.end(), 0);
		}

		template<typename T>
		static auto min(const Vector<T>& v1, const Vector<T>& v2)
		{
			Vector<typename T::CONSTUCTIBLE_TYPE> result;

			for (int i = 0; i < N; ++i)
				result[i] = std::min(v1[i], v2[i]);

			return result;
		}

		template<typename T>
		static	auto max(const Vector<T>& v1, const Vector<T>& v2)
		{
			Vector<typename T::CONSTUCTIBLE_TYPE> result;

			for (int i = 0; i < N; ++i)
				result[i] = std::max(v1[i], v2[i]);

			return result;
		}

		template<typename T>
		auto operator>(const Vector<T>& v2) const
		{
			Vector<typename T::CONSTUCTIBLE_TYPE> result;

			for (int i = 0; i < N; ++i)
				result[i] = values[i] > v2[i];

			return result;
		}

		template<typename T>
		auto operator<(const Vector<T>& v2) const {
			Vector<typename T::CONSTUCTIBLE_TYPE> result;

			for (int i = 0; i < N; ++i)
				result[i] = values[i] < v2[i];

			return result;
		}


		template<typename T>
		auto operator>=(const Vector<T>& v2) const
		{
			Vector<typename T::CONSTUCTIBLE_TYPE> result;

			for (int i = 0; i < N; ++i)
				result[i] = values[i] >= v2[i];

			return result;
		}

		template<typename T>
		auto operator<=(const Vector<T>& v2) const {
			Vector<typename T::CONSTUCTIBLE_TYPE> result;

			for (int i = 0; i < N; ++i)
				result[i] = values[i] <= v2[i];

			return result;
		}


		template<typename T>
		static	auto abs(const Vector<T>& v1)
		{
			Vector<typename T::CONSTUCTIBLE_TYPE> result;

			for (int i = 0; i < N; ++i)
				result[i] = fabs(v1[i]);

			return result;
		}

		template<typename T>
		static typename T::Format dot(const Vector<T>& a, const Vector<T>& b)
		{
			typename T::Format r = typename T::Format();

			for (int i = 0; i < T::N; i++)
				r += a[i] * b[i];

			return r;
		}


		static	Vector<T> cross(Vector<T> v1, Vector<T> v2)
		{
			static_assert(T::N == 3 || T::N == 4, "cross function cant be used here");
			// x  y  z
			// ax ay az
			// bx by bz
			// x = ay*bz - by*az
			// y = -(ax*bz - az*bx) = az*bx - ax*bz
			// z = ax*by - bx*ay
			return Vector<T>(v1.y * v2.z - v2.y * v1.z, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v2.x * v1.y);
		}


	};





	template<typename T>
	bool operator==(const Vector<T>& a, const Vector<T>& b)
	{
		for (unsigned int i = 0; i < T::N; ++i)
			if (a[i] != b[i])
				return false;

		return true;
	}

	template<typename T>
	bool operator!=(const Vector<T>& a, const Vector<T>& b)
	{
		for (unsigned int i = 0; i < T::N; ++i)
			if (a[i] != b[i])
				return true;

		return false;
	}

	template<typename T, typename T2, typename std::enable_if < std::is_scalar <T2>::value >::type* = nullptr >
	Vector<T> operator*(T2 n, Vector<T> v)
	{
		for (int i = 0; i < T::N; ++i)
			v[i] = static_cast<typename T::Format>(n * v[i]);

		return v;
	}

	template<typename T, typename T2>
	auto operator*(const Vector<T>& a, const Vector<T2>& b)
	{
		Vector<typename T::CONSTUCTIBLE_TYPE> v;

		for (int i = 0; i < T::N; ++i)
			v[i] = a[i] * b[i];

		return v;
	}

	template<typename T, typename T2>
	auto operator/(const Vector<T>& a, const Vector<T2>& b)
	{
		Vector<typename T::CONSTUCTIBLE_TYPE> v;

		for (int i = 0; i < T::N; ++i)
			v[i] = a[i] / b[i];

		return v;
	}

	template<typename T, typename T2, typename std::enable_if < std::is_scalar <T2>::value >::type* = nullptr >
	Vector<T> operator*(Vector<T> v, T2 n)
	{
		for (int i = 0; i < T::N; ++i)
			v[i] = static_cast<typename T::Format>(n * v[i]);
		return v;
	}

	template<typename T, typename T2, typename std::enable_if < std::is_scalar <T2>::value >::type* = nullptr >
	Vector<T> operator/(T2 n, Vector<T> v)
	{
		for (int i = 0; i < T::N; ++i)
			v[i] = n / v[i];

		return v;
	}

	template<typename T, typename T2, typename std::enable_if < std::is_scalar <T2>::value >::type* = nullptr >
	Vector<T> operator/(Vector<T> v, T2 n)
	{
		for (int i = 0; i < T::N; ++i)
			v[i] = static_cast<typename T::Format>(static_cast<T2>(v[i]) / n);

		return v;
	}

	// Linear intERPolation
	template <typename T>
	T lerp(T& p0, T& p1, float t)
	{
		return T((1 - t) * p0 + t * p1);
	}


	// Spherical Linear intERPolation
	template <typename T>
	T slerp(T& p0, T& p1, float t)
	{
		float cosom = dot(p0, p1);

		if (1 - fabs(cosom) >= eps2)
		{
			// slerp
			float omega = acos(cosom);
			float isinom = 1.0f / sin(omega);
			float coef1 = sin((1 - t) * omega) * isinom;
			float coef2 = sin(t * omega) * isinom;
			return coef1 * p0 + coef2 * p1;
		}

		else
		{
			// lerp to avoid devision by zero
			T r = lerp(p0, p1, t);
			r->norm();
			return r;
		}
	}



	template<typename T, bool CONSTRUCTIBLE = true >
	struct vec2_t
	{
		typedef T Format;
		static const int N = 2;
		static const bool GENERATE_CONSTRUCTOR = CONSTRUCTIBLE;
		using CONSTUCTIBLE_TYPE = vec2_t<T>;

		union
		{
			struct
			{
				std::array<T, 2> values;
			};
			struct
			{
				T x, y;
			};
		};
		template<typename = typename std::enable_if_t<GENERATE_CONSTRUCTOR>>
		vec2_t() {};
	};

	template<typename T, bool CONSTRUCTIBLE = true >
	struct vec3_t
	{
		typedef T Format;
		static const int N = 3;
		static const bool GENERATE_CONSTRUCTOR = CONSTRUCTIBLE;
		using CONSTUCTIBLE_TYPE = vec3_t<T>;

		union
		{
			struct
			{
				std::array<T, 3>  values;
			};
			struct
			{
				T x;
				union
				{
					Vector<vec2_t<T, false>> yz;
					struct
					{
						T y, z;
					};
				};
			};
			struct
			{
				Vector<vec2_t<T, false>> xy;
			};
		};
		template<typename = typename std::enable_if_t<GENERATE_CONSTRUCTOR>>
		vec3_t() {};
	};

	template<typename T, bool CONSTRUCTIBLE = true >
	struct vec4_t
	{
		typedef T Format;
		static const int N = 4;
		static const bool GENERATE_CONSTRUCTOR = CONSTRUCTIBLE;
		using CONSTUCTIBLE_TYPE = vec4_t<T>;

		union
		{
			struct
			{
				std::array<T, 4> values;
			};
			struct
			{
				union
				{
					Vector<vec3_t<T, false>> xyz;
					struct
					{
						T x;
						union
						{
							Vector<vec2_t<T, false>>  yz;
							struct
							{
								T y, z;
							};
						};
					};
				};
				T w;
			};
			struct
			{
				Vector<vec2_t<T, false>> xy;
				Vector<vec2_t<T, false>> zw;
			};
		};
		template<typename = typename std::enable_if_t<GENERATE_CONSTRUCTOR>>
		vec4_t() {};

	};


	template<typename T, int _N, bool CONSTRUCTIBLE = true >
	struct vecN_t
	{
		typedef T Format;
		static const int N = _N;
		static const bool GENERATE_CONSTRUCTOR = CONSTRUCTIBLE;
		using CONSTUCTIBLE_TYPE = vecN_t<T, _N>;

		union
		{
			struct
			{
				std::array<T, _N>  values;
			};

		};
		template<typename = typename std::enable_if_t<GENERATE_CONSTRUCTOR>>
		vecN_t() {};
	};

	template<typename T, bool CONSTRUCTIBLE = true >
	struct box_t
	{
		typedef T Format;
		static const int N = 6;
		static const bool GENERATE_CONSTRUCTOR = CONSTRUCTIBLE;

		using CONSTUCTIBLE_TYPE = box_t<T>;

		union
		{
			struct
			{
				std::array<T, 6>  values;
			};

			struct
			{
				T left;
				T top;
				T znear;


				T right;
				T bottom;
				T zfar;

			};

			struct
			{
				Vector<vec3_t<T, false>> a;
				Vector<vec3_t<T, false>> b;
			};
		};

		template<typename = typename std::enable_if_t<GENERATE_CONSTRUCTOR>>
		box_t() {};
	};

	template<typename T, bool CONSTRUCTIBLE = true >
	struct rect_t
	{
		typedef T Format;
		static const int N = 4;
		static const bool GENERATE_CONSTRUCTOR = CONSTRUCTIBLE;

		using CONSTUCTIBLE_TYPE = rect_t<T>;

		union
		{
			struct
			{
				std::array<T, 4> values;
			};
			struct
			{
				T x;
				T y;
				T w;
				T h;
			};
			struct
			{
				Vector<vec2_t<T, false>> pos;
				Vector<vec2_t<T, false>> size;
			};
		};

		template<typename = typename std::enable_if_t<GENERATE_CONSTRUCTOR>>
		rect_t() {};
	};

	template<typename T, bool CONSTRUCTIBLE = true >
	struct margin_t
	{
		typedef T Format;
		static const int N = 4;
		static const bool GENERATE_CONSTRUCTOR = CONSTRUCTIBLE;
		using CONSTUCTIBLE_TYPE = margin_t<T>;

		union
		{
			struct
			{
				std::array<T, 4> values;
			};
			struct
			{
				T left;
				T top;
				T right;
				T bottom;
			};
			struct
			{
				Vector<vec2_t<T, false>> left_top;
				Vector<vec2_t<T, false>> right_bottom;
			};
		};
		template<typename = typename std::enable_if_t<GENERATE_CONSTRUCTOR>>
		margin_t() {};
	};

	template<typename T, int Count, bool CONSTRUCTIBLE = true >
	struct vector_data_t
	{
		typedef T Format;
		static const int N = Count;

		static const bool GENERATE_CONSTRUCTOR = CONSTRUCTIBLE;
		using CONSTUCTIBLE_TYPE = vector_data_t<T, Count >;


		std::array<T, Count> values;

		vector_data_t() {};
	};


	typedef Vector<vec4_t<float>> vec4;
	typedef Vector<vec3_t<float>> vec3;
	typedef Vector<vec2_t<float>> vec2;

	typedef vec2 float2;
	typedef vec3 float3;
	typedef vec4 float4;

	typedef Vector<vec4_t<int>> ivec4;
	typedef Vector<vec3_t<int>> ivec3;
	typedef Vector<vec2_t<int>> ivec2;

	typedef ivec2 int2;
	typedef ivec3 int3;
	typedef ivec4 int4;


	typedef Vector<box_t<float>> box;
	typedef Vector<rect_t<float>> rect;
	typedef Vector<margin_t<float>> sizer;
	typedef Vector<margin_t<long>> sizer_long;

	typedef Vector<vec4_t<unsigned char>> rgba8;
	typedef Vector<vec3_t<unsigned char>> rgb8;


	using uint = unsigned int;
	using uint2 = ivec2;
	using uint3 = ivec3;
	using uint4 = ivec4;



	vec3 closest_point_on_line(const vec3& a, const vec3& b, const vec3& p)
	{
		vec3 c = p - a;
		vec3 V = b - a;
		float d = V.length();
		V.normalize();
		float t = vec3::dot(V, c);		// скалярное произведение векторов

		// проверка на выход за границы отрезка
		if (t < 0.0f)
			return a;

		if (t > d)
			return b;

		// Вернем точку между a и b
		V *= t;
		return (a + V);
	}

	vec3 ortogonalize(const vec3& v1, const vec3& v2)
	{
		vec3 v2ProjV1 = closest_point_on_line(v1, -v1, v2);
		vec3 res = v2 - v2ProjV1;
		res.normalize();
		return res;
	}

	sizer intersect(const sizer& a, const sizer& b)
	{
		float x = std::max(a.left, b.left);
		float num1 = std::min(a.right, b.right);
		float y = std::max(a.top, b.top);
		float num2 = std::min(a.bottom, b.bottom);

		if (num1 >= x && num2 >= y)
			return	sizer(x, y, num1, num2);

		return sizer(0, 0, 0, 0);
	}

	rect intersect(const rect& a, const rect& b)
	{
		float x = std::max(a.pos.x, b.pos.x);
		float num1 = std::min(a.pos.x + a.size.x, b.pos.x + b.size.x);
		float y = std::max(a.pos.y, b.pos.y);
		float num2 = std::min(a.pos.y + a.size.y, b.pos.y + b.size.y);

		if (num1 >= x && num2 >= y)
			return	rect(x, y, num1 - x, num2 - y);

		return rect(0, 0, 0, 0);
	}

namespace math
{

	sizer convert(const rect& b)
	{
		sizer a;
		a.left = b.x;
		a.top = b.y;
		a.right = b.x + b.w;
		a.bottom = b.y + b.h;
		return a;
	}

	rect convert(const sizer& b)
	{
		rect a;
		a.x = b.left;
		a.y = b.top;
		a.w = b.right - b.left;
		a.h = b.bottom - b.top;
		return a;
	}


	template <typename T>
	bool any(const Vector<T>& v)
	{
		for (auto& e : v.values)
			if (e > 0)
				return true;

		return false;
	}


	template <typename T>
	bool all(const Vector<T>& v)
	{
		for (auto& e : v.values)
			if (e == 0)
				return false;

		return true;
	}


}

	/*
	template <class T>
	concept VectorType =
	requires (T a) {
		T::VECTOR;
	};
	*/
	template<class V, class T>
	class grid
	{
		V grid_size;
		V scalers;
		std::vector<T> data;

	public:

		const V& size() const
		{
			return grid_size;
		}
		void resize(V size, T v = T())
		{
			grid_size = size;

			typename V::Format sum = 1;
			for (int i = 0; i < V::N; ++i)
			{
				scalers[i] = sum;
				sum *= size[i];
			}

			data.resize(sum, v);
		}

		void fill(T v)
		{
			std::fill(data.begin(), data.end(), v);
		}

		decltype(auto) operator[](const V& i)
		{
			return data[V::dot(i, scalers)];
		}

		auto begin()
		{
			return data.begin();
		}

		auto end()
		{
			return data.end();
		}
	};


	template<class T>
	void serialize(simple_log_archive& ar, Vector<vec2_t<T>>& a, const unsigned int)
	{
		ar& NP("x", a.x)
			& NP("y", a.y);
	}

	template<class T>
	void serialize(simple_log_archive& ar, Vector<vec3_t<T>>& a, const unsigned int)
	{
		ar& NP("x", a.x)
			& NP("y", a.y)
			& NP("z", a.z);
	}

	template<class T>
	void serialize(simple_log_archive& ar, Vector<vec4_t<T>>& a, const unsigned int)
	{
		ar& NP("x", a.x)
			& NP("y", a.y)
			& NP("z", a.z)
			& NP("w", a.w);
	}

	template<class Archive, class T>
	void serialize(Archive& ar, Vector<T>& a, const unsigned int)
	{
		ar& boost::serialization::make_array(a.values.data(), T::N);
	}
}
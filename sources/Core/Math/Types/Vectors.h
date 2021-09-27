#pragma once
#include "Math/Constants.h"
#include "Serialization/serialization.h"

namespace internal
{

	template<typename T> concept IsVectorType =
		requires () {
		T::N;
	};


	template<class T>
	consteval int get_count() requires (!IsVectorType<T>)
	{
		return 0xff;
	}

	template<class T>
	consteval int get_count() requires (IsVectorType<T>)
	{
		return T::N;
	}

	template< class ...Args>
	consteval int calc_count()
	{
		return (0 + ... + get_count<Args>());
	}

}


// Basic vector
template<typename T>
class Vector : public T
{
public:
	using T::values;
	using Format = typename T::Format;
	using T::N;
	using raw = typename T;
private:


	template<int i, class D, class ...Args>
	void set_internal(D d, Args ...args) requires (std::is_compound_v<D>)
	{
		constexpr int size = std::min(N, D::N);
		for (int t : view::iota(0, size))
			values[i + t] = static_cast<Format>(d.values[t]);

		set_internal<i + size>(args...);
	}

	template<int i, class D, class ...Args>
	void set_internal(D d, Args ...args) requires (!std::is_compound_v<D>)
	{
		values[i] = static_cast<Format>(d);
		set_internal<i + 1>(args...);
	}


	template<int i>
	void set_internal()
	{
		//static_assert(i == T::N);
	}

public:

	Vector() requires(T::GENERATE_CONSTRUCTOR)
	{
		memset(T::values.data(), 0, sizeof(Format) * T::N);
	}


	template<class ...Args>
	Vector(Args ...args) requires(T::GENERATE_CONSTRUCTOR && (internal::calc_count<Args...>() >= N))
	{
		set(args...);
	}

	template<class V>
	Vector(V v) requires(T::GENERATE_CONSTRUCTOR && (!std::is_compound_v<V>))
	{
		set(v);
	}


	template<class ...Args>
	void set(Args ...args)  requires (internal::calc_count<Args...>() >= N)
	{
		set_internal<0>(args...);
	}

	template<class V>
	void set(V v) requires (!std::is_compound_v<V>)
	{
		for (int t : view::iota(0, N))
			T::values[t] = static_cast<Format>(v);
	}

	Format* data()
	{
		return this->values.data();
	}

	Format& operator[](unsigned int i) { return this->values[i]; }
	const Format& operator[](unsigned int i) const { return this->values[i]; }

	template<class T2>
	Vector& operator=(const Vector<T2>& v) requires(N == T2::N)
	{
		for (int i : view::iota(0, N))
			values[i] = static_cast<Format>(v[i]);
		return *this;
	}

	template<class T2>
	inline Vector operator+(const Vector<T2>& v) const requires(N == T2::N)
	{
		Vector result;

		for (int i : view::iota(0, N))
			result[i] = values[i] + v[i];

		return result;
	}

	template<class T2>
	inline Vector operator-(const Vector<T2>& v) const requires(N == T2::N)
	{
		Vector result(*this);

		for (int i : view::iota(0, N))
			result[i] = values[i] - v[i];

		return result;
	}

	inline Vector operator-() const
	{
		Vector<T> result;

		for (int i : view::iota(0, N))
			result[i] = -values[i];

		return result;
	}
	template<class T2>
	inline Vector& operator+=(const Vector<T2>& v) requires(N == T2::N)
	{
		for (int i : view::iota(0, N))
			values[i] += v[i];

		return *this;
	}
	template<class T2>
	inline Vector& operator-=(const Vector<T2>& v) requires(N == T2::N)
	{
		for (int i : view::iota(0, N))
			values[i] -= v[i];

		return *this;
	}

	inline Vector& operator*=(const Format& n) 
	{
		for (int i : view::iota(0, N))
			values[i] *= n;

		return *this;
	}

	inline Vector& operator/=(const Format& n)
	{
		for (int i : view::iota(0, N))
			values[i] /= n;

		return *this;
	}

	inline Format length_squared() const
	{
		return dot(*this, *this);
	}

	inline auto length() const
	{
		return sqrt(length_squared());
	}

	Vector<T>& normalize(float eps = Math::eps12)
	{
		auto d = length_squared();

		if (d > eps)
			(*this) /= sqrt(d);

		return *this;
	}


	Vector<T> normalized(float eps = Math::eps12) const
	{
		Vector res(*this);
		return res.normalize();
	}

	Format max_element() const
	{
		return *std::max_element(values.begin(), values.end());
	}

	Format sum() const
	{
		return std::accumulate(values.begin(), values.end(), Format(0));
	}



	auto abs() const
	{
		Vector<typename T::CONSTUCTIBLE_TYPE> result;

		for (int i : view::iota(0, N))
			result[i] = std::abs(values[i]);
		return result;
	}
	template<typename T>
	static auto min(const Vector<T>& v1, const Vector<T>& v2) 
	{
		Vector<typename T::CONSTUCTIBLE_TYPE> result;

		for (int i : view::iota(0, N))
			result[i] = std::min(v1[i], v2[i]);

		return result;
	}

	template<typename T>
	static	auto max(const Vector<T>& v1, const Vector<T>& v2)
	{
		Vector<typename T::CONSTUCTIBLE_TYPE> result;

		for (int i : view::iota(0, N))
			result[i] = std::max(v1[i], v2[i]);

		return result;
	}

	template<typename T>
	static	auto clamp(const Vector<T>& v, const Vector<T>& _min, const Vector<T>& _max)
	{
		Vector<typename T::CONSTUCTIBLE_TYPE> result;

		for (int i : view::iota(0, N))
			result[i] = std::clamp(v[i], _min[i], _max[i]);

		return result;
	}

	template<typename T>
	auto operator>(const Vector<T>& v2) const
	{
		Vector<typename T::CONSTUCTIBLE_TYPE> result;

		for (int i : view::iota(0, N))
			result[i] = values[i] > v2[i];

		return result;
	}

	template<typename T>
	auto operator<(const Vector<T>& v2) const {
		Vector<typename T::CONSTUCTIBLE_TYPE> result;

		for (int i : view::iota(0, N))
			result[i] = values[i] < v2[i];

		return result;
	}


	template<typename T>
	auto operator>=(const Vector<T>& v2) const
	{
		Vector<typename T::CONSTUCTIBLE_TYPE> result;

		for (int i : view::iota(0, N))
			result[i] = values[i] >= v2[i];

		return result;
	}

	template<typename T>
	auto operator<=(const Vector<T>& v2) const {
		Vector<typename T::CONSTUCTIBLE_TYPE> result;

		for (int i : view::iota(0, N))
			result[i] = values[i] <= v2[i];

		return result;
	}


	template<typename T>
	static	auto abs(const Vector<T>& v1)
	{
		Vector<typename T::CONSTUCTIBLE_TYPE> result;

		for (int i : view::iota(0, N))
			result[i] = fabs(v1[i]);

		return result;
	}

	template<typename T>
	static Format dot(const Vector<T>& a, const Vector<T>& b)
	{
		Format r(0);

		for (int i : view::iota(0, N))
			r += a[i] * b[i];

		return r;
	}


	static	Vector<T> cross(Vector<T> v1, Vector<T> v2) requires(T::N == 3)
	{
		return Vector<T>(v1.y * v2.z - v2.y * v1.z, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v2.x * v1.y);
	}

	private:

		SERIALIZE_PRETTY() requires (N <= 4)
		{
			if constexpr (N > 0)ar& NP("x", values[0]);
			if constexpr (N > 1)ar& NP("y", values[1]);
			if constexpr (N > 2)ar& NP("z", values[2]);
			if constexpr (N > 3)ar& NP("w", values[3]);
		}


		SERIALIZE()
		{
			ar& NP("values", values);
		}

	
};

template<typename T>
bool operator==(const Vector<T>& a, const Vector<T>& b)
{
	for (int i : view::iota(0, T::N))
		if (a[i] != b[i])
			return false;

	return true;
}

template<typename T>
bool operator!=(const Vector<T>& a, const Vector<T>& b)
{
	for (int i : view::iota(0, T::N))
		if (a[i] != b[i])
			return true;

	return false;
}

template<typename T, typename T2>
Vector<T> operator*(T2 n, Vector<T> v) requires(std::is_scalar_v<T2>)
{
	for (int i : view::iota(0, T::N))
		v[i] = static_cast<Vector<T>::Format>(n * v[i]);

	return v;
}

template<typename T, typename T2>
auto operator*(const Vector<T>& a, const Vector<T2>& b)  requires(T::N == T2::N)
{
	Vector<typename T::CONSTUCTIBLE_TYPE> v;

	for (int i : view::iota(0, T::N))
		v[i] = a[i] * b[i];

	return v;
}

template<typename T, typename T2>
auto operator/(const Vector<T>& a, const Vector<T2>& b)  requires(T::N == T2::N)
{
	Vector<typename T::CONSTUCTIBLE_TYPE> v;

	for (int i : view::iota(0, T::N))
		v[i] = a[i] / b[i];

	return v;
}

template<typename T, typename T2>
Vector<T> operator*(Vector<T> v, T2 n) requires(std::is_scalar_v<T2>)
{
	for (int i : view::iota(0, T::N))
		v[i] = static_cast<Vector<T>::Format>(n * v[i]);
	return v;
}

template<typename T, typename T2>
Vector<T> operator/(T2 n, Vector<T> v)  requires(std::is_scalar_v<T2>)
{
	for (int i : view::iota(0, T::N))
		v[i] = n / v[i];

	return v;
}

template<typename T, typename T2>
Vector<T> operator/(Vector<T> v, T2 n) requires(std::is_scalar_v<T2>)
{
	for (int i : view::iota(0, T::N))
		v[i] = static_cast<T::Format>(static_cast<T2>(v[i]) / n);

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

	if (1 - fabs(cosom) >= Math::eps2)
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

	vec2_t() requires(GENERATE_CONSTRUCTOR) {};
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


	vec3_t() requires(GENERATE_CONSTRUCTOR) {}
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

	vec4_t() requires(GENERATE_CONSTRUCTOR) {}
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

	box_t() requires(GENERATE_CONSTRUCTOR) {}
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

	rect_t() requires(GENERATE_CONSTRUCTOR) {}
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

	margin_t() requires(GENERATE_CONSTRUCTOR) {}
};

template<typename T, int Count, bool CONSTRUCTIBLE = true >
struct vector_data_t
{
	typedef T Format;
	static const int N = Count;

	static const bool GENERATE_CONSTRUCTOR = CONSTRUCTIBLE;
	using CONSTUCTIBLE_TYPE = vector_data_t<T, Count >;

	std::array<T, Count> values;

	vector_data_t() requires(GENERATE_CONSTRUCTOR) {}
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


using uint = UINT;
using uint2 = Vector<vec2_t<uint>>;
using uint3 = Vector<vec3_t<uint>>;
using uint4 = Vector<vec4_t<uint>>;



sizer intersect(const sizer& a, const sizer& b);
rect intersect(const rect& a, const rect& b);

vec3 closest_point_on_line(const vec3& a, const vec3& b, const vec3& p);

vec3 ortogonalize(const vec3& v1, const vec3& v2);
namespace math
{
	sizer convert(const rect& b);
	rect convert(const sizer& b);


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

		size_t sum = 1;
		for (int i : view::iota(0, V::N))
		{
			scalers[i] = static_cast<V::Format>(sum);
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
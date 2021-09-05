#pragma once

#pragma warning(disable:4201)
#pragma warning(disable:4520)

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
		for (int t = 0; t < size; t++)
			values[i + t] = static_cast<Format>(d.values[t]);

		set_internal<i + size>(args...);
	}

	template<int i, class D, class ...Args>
	void set_internal(D d, Args ...args)requires (!std::is_compound_v<D>)
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
	Vector(Args ...args) requires(T::GENERATE_CONSTRUCTOR)
	{
		set(args...);
	}

	template<class ...Args>
	void set(Args ...args)
	{
		set_internal<0>(args...);
	}

	template<class V>
	void set(V v) requires (!std::is_compound_v<V>)
	{
		for (int t = 0; t < N; t++)
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
		for (int i = 0; i < T::N; ++i)
			values[i] = static_cast<Format>(v[i]);
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

	inline Vector& operator*=(const Format& n)
	{
		for (int i = 0; i < T::N; ++i)
			values[i] *= n;

		return *this;
	}

	inline Vector& operator/=(const Format& n)
	{
		for (int i = 0; i < T::N; ++i)
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


	Vector<T> normalized(float eps = Math::eps12)
	{
		Vector res(*this);
		return res.normalize();
	}

	Format max_element()
	{
		return *std::max_element(values.begin(), values.end());
	}

	Format sum()
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
	static Format dot(const Vector<T>& a, const Vector<T>& b)
	{
		Format r = 0.0;

		for (int i = 0; i < T::N; i++)
			r += a[i] * b[i];

		return r;
	}


	static	Vector<T> cross(Vector<T> v1, Vector<T> v2) requires(T::N == 3)
	{
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

template<typename T, typename T2>
Vector<T> operator*(T2 n, Vector<T> v) requires(std::is_scalar_v<T2>)
{
	for (int i = 0; i < T::N; ++i)
		v[i] = static_cast<Vector<T>::Format>(n * v[i]);

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

template<typename T, typename T2>
Vector<T> operator*(Vector<T> v, T2 n) requires(std::is_scalar_v<T2>)
{
	for (int i = 0; i < T::N; ++i)
		v[i] = static_cast<Vector<T>::Format>(n * v[i]);
	return v;
}

template<typename T, typename T2>
Vector<T> operator/(T2 n, Vector<T> v)  requires(std::is_scalar_v<T2>)
{
	for (int i = 0; i < T::N; ++i)
		v[i] = n / v[i];

	return v;
}

template<typename T, typename T2>
Vector<T> operator/(Vector<T> v, T2 n) requires(std::is_scalar_v<T2>)
{
	for (int i = 0; i < T::N; ++i)
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
using uint2 = ivec2;
using uint3 = ivec3;
using uint4 = ivec4;



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
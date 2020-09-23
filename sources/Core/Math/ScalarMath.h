namespace Math
{

    const float eps2 = 0.001f;
    const float eps4 = 0.00001f;
    const float eps6 = 0.0000001f;
    const float eps8 = 0.000000001f;
    const float eps10 = 0.00000000001f;
    const float eps12 = 0.0000000000001f;

    // math constants
    const float m_pi = 3.141592653589793238462643383279f;
    const float m_pi_2 = m_pi / 2.0f;
    const float m_2_pi = m_pi * 2.0f;

    const float e = 2.718281828459045235360287471352f;
    const float radToDeg = 180 / m_pi;
    const float degToRad = m_pi / 180;

    // math special functions
    template<class T>
    T clamp(const T& n, const T& min, const T& max)
    {
        return std::min(std::max(n, min), max);
    }

    template<class T>
    T saturate(const T& n)
    {
        return clamp(n, static_cast<T>(0), static_cast<T>(1));
    }

    float sin(const float angle);

    float cos(const float angle);


    template <class A, class B>
    auto pow(A a, B b)
    {
        return std::pow(a, b);
    }

    template <typename T> __forceinline T AlignUpWithMask(T value, size_t mask)
    {
        return (T)(((size_t)value + mask) & ~mask);
    }

    template <typename T> __forceinline T AlignDownWithMask(T value, size_t mask)
    {
        return (T)((size_t)value & ~mask);
    }

    template <typename T> __forceinline T AlignUp(T value, size_t alignment)
    {
        return AlignUpWithMask(value, alignment - 1);
    }

    template <typename T> __forceinline T roundUp(T a, size_t b) { return (1 + (a - 1) / b) * b; }

    template <typename T> __forceinline T AlignDown(T value, size_t alignment)
    {
        return AlignDownWithMask(value, alignment - 1);
    }

    template <typename T> __forceinline bool IsAligned(T value, size_t alignment)
    {
        return 0 == (size_t(value) & (alignment - 1));
    }

    template <typename T> __forceinline T DivideByMultiple(T value, size_t alignment)
    {
        return (T)((value + alignment - 1) / alignment);
    }


}
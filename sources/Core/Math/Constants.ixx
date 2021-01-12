
#include <algorithm>

export module Constants;

export
{

    // precision constants

    const float eps2 = 0.001f;
    const float eps4 = 0.00001f;
    const float eps6 = 0.0000001f;
    const float eps8 = 0.000000001f;
    const float eps10 = 0.00000000001f;
    const float eps12 = 0.0000000000001f;

    // math constants
    const float pi = 3.141592653589793238462643383279f;
    const float e = 2.718281828459045235360287471352f;
    const float radToDeg = 180 / pi;
    const float degToRad = pi / 180;
}

// math special functions
export template<class T>
T saturate(const T& n)
{
    return std::min(std::max(n, static_cast<T>(0)), static_cast<T>(1));
}

#include "pch.h"
#include "Vectors.h"

vec3 closest_point_on_line(const vec3 &a, const vec3 &b, const vec3 &p)
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

vec3 ortogonalize(const vec3 &v1, const vec3 &v2)
{
    vec3 v2ProjV1 = closest_point_on_line(v1, -v1, v2);
    vec3 res = v2 - v2ProjV1;
    res.normalize();
    return res;
}

sizer intersect(const sizer &a, const sizer &b)
{
    float x = std::max(a.left, b.left);
	float num1 = std::min(a.right, b.right);
	float y = std::max(a.top, b.top);
	float num2 = std::min(a.bottom, b.bottom);

    if (num1 >= x && num2 >= y)
        return	sizer(x, y, num1, num2);

    return sizer(0, 0, 0, 0);
}

rect intersect(const rect &a, const rect &b)
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

    sizer convert(const rect &b)
    {
        sizer a;
        a.left = b.x;
        a.top = b.y;
        a.right = b.x + b.w;
        a.bottom = b.y + b.h;
        return a;
    }

    rect convert(const sizer &b)
    {
        rect a;
        a.x = b.left;
        a.y = b.top;
        a.w = b.right - b.left;
        a.h = b.bottom - b.top;
        return a;
    }
}
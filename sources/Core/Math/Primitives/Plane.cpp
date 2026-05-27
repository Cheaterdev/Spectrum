module Core:Math.Plane;

import :Math.Constants;
import :Math.Vectors;
import :Math.Quaternion;
import :Math.Matrices;

Plane::Plane(vec3 n, vec3 p)
{
    this->n = n;
    this->d = -vec3::dot(n, p);
    origin = p;
}

Plane::Plane(vec3 n, float d)
{
    this->n = n;
    this->d = d;
    origin = -n * (*this)(vec3(0, 0, 0));
}

Plane::Plane(vec3 a, vec3 b, vec3 c)
{
    this->n = vec3::cross(a - b, a - c).normalize();
    this->d = -vec3::dot(n, a);
    origin = a;
}

void Plane::normalize()
{
    float iL = 1.0f / n.length();
    n *= iL;
    d *= iL;
}

float Plane::operator()(const vec3& p) const
{
    return vec3::dot(n, p) + d;
}

void Plane::ApplyTransform(const mat4x4& m)
{
    vec3 p = d * n;
    n = (vec4(n, 0) * m).xyz;
    p = (vec4(p, 1) * m).xyz;
    n.normalize();
    d = -vec3::dot(n, p);
}

Plane::operator vec4() const
{
    return vec4(n, d);
}

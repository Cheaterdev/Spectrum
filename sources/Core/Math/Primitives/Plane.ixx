module;
#include "Serialization/serialization_defines.h"
export module Plane;

export import Constants;
export import Vectors;
export import Quaternion;
export import Matrices;
import stl.memory;
import serialization;

export class Plane
{
public:
    Plane() = default;
    Plane(vec3 n, vec3 p);
    Plane(vec3 n, float d);
    Plane(vec3 a, vec3 b, vec3 c);

    void normalize();
    float operator()(const vec3& p) const;
    void ApplyTransform(const mat4x4& m);

    operator vec4() const;
    vec3 n;
    float d;

    vec3 origin;

private:
    SERIALIZE()
    {
        ar& NVP(n)& NVP(d)& NVP(origin);
    }
};


module: private;


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
    n = vec3(vec4(n, 0) * m);
    p = vec3(vec4(p, 1) * m);
    n.normalize();
    d = -vec3::dot(n, p);
}

Plane::operator Vector<vec4_t<float>>() const
{
    return vec4(n, d);
}


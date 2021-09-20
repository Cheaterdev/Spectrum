#pragma once
#include "Primitive.h"

class Plane
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
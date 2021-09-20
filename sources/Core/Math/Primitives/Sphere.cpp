#include "pch.h"
#include "Sphere.h"

// Sphere block
Sphere::Sphere(vec3 pos, float radius)
{
    this->pos = pos;
    this->radius = radius;
}

float Sphere::GetSurfaceArea() const
{
    const float cfArea = 4.0f * Math::pi;
    return cfArea * radius * radius;
}

BOOST_CLASS_EXPORT(Sphere)
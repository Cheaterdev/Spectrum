#pragma once

#include "AABB.h"
#include "Frustum.h"
#include "Plane.h"
#include "Ray.h"
#include "Sphere.h"



enum INTERSECT_TYPE
{
    FULL_OUT = 0,
    FULL_IN = 1,
    IN_NEAR = 2
};

INTERSECT_TYPE intersect(const Frustum& f, const Sphere& s, const mat4x4* mat=nullptr);
INTERSECT_TYPE intersect(const Frustum& f, const AABB& aabb, const mat4x4* mat = nullptr);
INTERSECT_TYPE intersect(const AABB* aabb1, const AABB& aabb2, const mat4x4* mat = nullptr);
INTERSECT_TYPE intersect(const Frustum& f, const Primitive* primitive, const mat4x4* mat = nullptr);


template<class T>
std::shared_ptr<Primitive> get_best_primitive(T& functor, unsigned int size)
{
    std::shared_ptr<AABB> aabb(new AABB(functor, size));
    //	std::shared_ptr<Sphere> sphere(new Sphere(functor, size));
    return aabb;
}

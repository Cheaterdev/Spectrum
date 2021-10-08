module;
#include "Serialization/serialization_defines.h"
export module Intersections;

export import Constants;
export import Vectors;
export import Quaternion;
export import Matrices;
export import Primitive;

import stl.memory;
import serialization;

import AABB;
import Frustum;
import Sphere;


export {
    enum INTERSECT_TYPE
    {
        FULL_OUT = 0,
        FULL_IN = 1,
        IN_NEAR = 2
    };

    INTERSECT_TYPE intersect(const Frustum& f, const Sphere& s, const mat4x4* mat = nullptr);
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

}


module: private;


INTERSECT_TYPE intersect(const Frustum& f, const Sphere& s, const mat4x4*)
{
    for (int i = 0; i < Frustum::planes::COUNT; ++i)
    {
        float d = f.GetFrustumPlane(Frustum::planes(i))(s.pos) - s.radius;

        if (d > 0) return INTERSECT_TYPE::FULL_OUT;
    }

    return INTERSECT_TYPE::FULL_IN;
}

INTERSECT_TYPE intersect(const Frustum& f, const AABB& aabb, const mat4x4* mat)
{
    vec3 p[8];
    p[0] = vec3(aabb.max.x, aabb.max.y, aabb.max.z);
    p[1] = vec3(aabb.max.x, aabb.max.y, aabb.min.z);
    p[2] = vec3(aabb.max.x, aabb.min.y, aabb.max.z);
    p[3] = vec3(aabb.max.x, aabb.min.y, aabb.min.z);
    p[4] = vec3(aabb.min.x, aabb.max.y, aabb.max.z);
    p[5] = vec3(aabb.min.x, aabb.max.y, aabb.min.z);
    p[6] = vec3(aabb.min.x, aabb.min.y, aabb.max.z);
    p[7] = vec3(aabb.min.x, aabb.min.y, aabb.min.z);


    if (mat) for (auto& el : p)
        el = el * *mat;
    bool out_near = false;

    for (int i = 0; i < Frustum::planes::COUNT; ++i)
    {

        bool bFullyOut = true;

        for (int j = 0; j < 8; ++j)
        {
            float d = f.GetFrustumPlane(Frustum::planes(i))(p[j]);

            if (d > 0)
                bFullyOut = false;
            else  if (i == 0)
                out_near = true;
        }

        if (bFullyOut)
            return INTERSECT_TYPE::FULL_OUT;
    }

    return out_near ? INTERSECT_TYPE::IN_NEAR : INTERSECT_TYPE::FULL_IN;
}

INTERSECT_TYPE intersect(const AABB& aabb1, const AABB& aabb2, const mat4x4*, float threshold)
{
    if (aabb1.max.x < aabb2.min.x - threshold || aabb2.max.x < aabb1.min.x - threshold) return INTERSECT_TYPE::FULL_OUT;

    if (aabb1.max.y < aabb2.min.y - threshold || aabb2.max.y < aabb1.min.y - threshold) return INTERSECT_TYPE::FULL_OUT;

    if (aabb1.max.z < aabb2.min.z - threshold || aabb2.max.z < aabb1.min.z - threshold) return INTERSECT_TYPE::FULL_OUT;
    else return INTERSECT_TYPE::FULL_IN;
}

INTERSECT_TYPE intersect(const Frustum& f, const Primitive* primitive, const mat4x4* mat)
{
    switch (primitive->get_type())
    {
    case primitive_types::sphere:
    {
        return intersect(f, *static_cast<const Sphere*>(primitive), mat);
    }

    case primitive_types::aabb:
    {
        return intersect(f, *static_cast<const AABB*>(primitive), mat);
    }
    }

    return INTERSECT_TYPE::FULL_OUT;
}
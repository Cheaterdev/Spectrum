#include "pch.h"
export module Geometry;

import Vectors;
import Quaternions;
import Matrices;
import Constants;

export
{


    enum class primitive_types
    {
        sphere,
        aabb,
        frustum,
        unknown
    };
    class Primitive
    {
    public:
        using ptr = std::shared_ptr<Primitive>;
        virtual const primitive_types get_type() const
        {
            return primitive_types::unknown;
        }

        virtual float get_volume() const
        {
            return 0;
        }

        virtual void combine(Primitive*)
        {
        }

        virtual void combine(Primitive*, mat4x4&)
        {
        }
        virtual vec3 get_min() = 0;
        virtual vec3 get_max() = 0;
        virtual void set(Primitive*, mat4x4&) {}
        virtual ~Primitive()
        {
        }
        virtual void apply_transform(ptr, mat4x4&) {};
        virtual ptr clone() = 0;
    };

    // dot(p,n)+d=0
    class Plane
    {
    public:
        Plane() {}
        Plane(vec3 n, vec3 p);
        Plane(vec3 n, float d);
        Plane(vec3 a, vec3 b, vec3 c);

        void normalize();
        float operator()(const vec3& p) const;
        void ApplyTransform(const mat4x4& m);

        operator vec4() const
        {
            return vec4(n, d);
        }
        vec3 n;
        float d;

        vec3 origin;
    };

    class Ray
    {
    public:

        vec3 pos;
        vec3 dir;

        Ray(vec3 p, vec3 r) : pos(p), dir(r)
        {
        }
        bool intersect(Plane& p, vec3& res)
        {
            float denom = vec3::dot(p.n, dir); // dot(planeNormal, getDirection());

            if (denom != 0.0f)
            {
                float dist = vec3::dot(p.n, p.origin - pos) / denom;
                res = pos + dist * dir;
                return true;
            }

            return false;
        }
    };
    class Frustum : public Primitive
    {
        vec3 global_points[8];
        mat4x4 inverse_mat;
    public:
        virtual std::shared_ptr<Primitive>clone()
        {
            return std::shared_ptr<Primitive>(new Frustum(*this));
        };
        virtual vec3 get_min()
        {
            vec3 min = global_points[0];

            for (auto& point : global_points)
                min = vec3::min(min, point);

            return min;
        };
        mat4x4 get_transform() const
        {
            return inverse_mat;
        }
        virtual vec3 get_max()
        {
            vec3 max = global_points[0];

            for (auto& point : global_points)
                max = vec3::max(max, point);

            return max;
        };

        virtual const primitive_types get_type() const
        {
            return primitive_types::frustum;
        }


        void normalize();

        void calculate(const mat4x4& inverse_mat);

        enum planes
        {
            plane_near,
            plane_far,
            plane_left,
            plane_right,
            plane_top,
            plane_bottom,

            COUNT
        };

        Plane& GetFrustumPlane(planes plane) { return p[plane]; }
        const Plane& GetFrustumPlane(planes plane) const { return p[plane]; }
    protected:
        Plane p[planes::COUNT];
    };


    class Sphere : public Primitive
    {
    public:
        virtual std::shared_ptr<Primitive>clone()
        {
            return std::shared_ptr<Primitive>(new Sphere(*this));
        };
        virtual const primitive_types get_type() const
        {
            return primitive_types::sphere;
        }

        Sphere() {}
        Sphere(vec3 pos, float radius);

        template<class T>
        Sphere(T& functor, unsigned int size)
        {
        }

        virtual float get_volume() const
        {
            const float cfVolume = 4.0f / 3.0f * Math::pi;
            return cfVolume * radius * radius * radius;
        }

        float GetSurfaceArea() const;

        vec3 pos;
        float radius;

        virtual vec3 get_min()
        {
            return pos - vec3(radius, radius, radius);
        }
        virtual vec3 get_max()
        {
            return pos + vec3(radius, radius, radius);
        }

        void apply_transform(Primitive::ptr p, mat4x4& m) override {

            pos = static_cast<Sphere*>(p.get())->pos * m;
            //scale is not supported now
        };
    };

    class AABB : public Primitive
    {
    public:
        virtual std::shared_ptr<Primitive>clone()
        {
            return std::shared_ptr<Primitive>(new AABB(*this));
        };
        virtual const primitive_types get_type() const
        {
            return primitive_types::aabb;
        }

        virtual float get_volume() const
        {
            vec3 size = max - min;
            return size.x * size.y * size.z;
        }
        AABB() { }
        AABB(Primitive* other)
        {
            min = other->get_min();
            max = other->get_max();
        }
        void set(Primitive* other)
        {
            min = other->get_min();
            max = other->get_max();
        }

        virtual void set(Primitive* other, mat4x4& m) override
        {
            vec3 a = other->get_min() * m;
            vec3 b = other->get_max() * m;
            min = vec3::min(a, b);
            max = vec3::max(a, b);
        }
        template<class T>
        AABB(T& functor, unsigned int size)
        {
            min = max = functor(0);

            for (unsigned int i = 1; i < size; i++)
            {
                vec3 pos = functor(i);
                min = vec3::min(min, pos);
                max = vec3::max(max, pos);
            }
        }
        virtual vec3 get_min()
        {
            return min;
        }
        virtual vec3 get_max()
        {
            return max;
        }

        virtual void combine(Primitive* other)
        {
            min = vec3::min(min, other->get_min());
            max = vec3::max(max, other->get_max());
        }

        virtual void combine(Primitive* other, mat4x4& m)
        {
            min = vec3::min(min, other->get_min() * m);
            max = vec3::max(max, other->get_max() * m);
        }


        void apply_transform(Primitive::ptr r, mat4x4& mi) override {

            auto& aabb = *static_cast<AABB*>(r.get());

            vec3 p[8];
            p[0] = vec3(aabb.max.x, aabb.max.y, aabb.max.z) * mi;
            p[1] = vec3(aabb.max.x, aabb.max.y, aabb.min.z) * mi;
            p[2] = vec3(aabb.max.x, aabb.min.y, aabb.max.z) * mi;
            p[3] = vec3(aabb.max.x, aabb.min.y, aabb.min.z) * mi;
            p[4] = vec3(aabb.min.x, aabb.max.y, aabb.max.z) * mi;
            p[5] = vec3(aabb.min.x, aabb.max.y, aabb.min.z) * mi;
            p[6] = vec3(aabb.min.x, aabb.min.y, aabb.max.z) * mi;
            p[7] = vec3(aabb.min.x, aabb.min.y, aabb.min.z) * mi;


            min = max = p[0];

            for (unsigned int i = 1; i < 8; i++)
            {
                vec3& pos = p[i];
                min = vec3::min(min, pos);
                max = vec3::max(max, pos);
            }
        };


        vec3 min, max;

        //     void FromMinMaxPoint(vec3 minPoint, vec3 maxPoint);
        //     __forceinline vec3 GetMinPoint() const { return pos - d; }
        //     __forceinline vec3 GetMaxPoint() const { return pos + d; }
        //     __forceinline float GetVolume() const { return 8 * d.x * d.y * d.z; }
    };

    class OBB
    {
    public:
        vec3 pos, ox, oy, oz;
        float dx, dy, dz;
    };

    enum INTERSECT_TYPE
    {
        FULL_OUT = 0,
        FULL_IN = 1,
        IN_NEAR = 2
    };

    // not quite right result (compound exclusion is not checked)
    // false - object fully exclude by one plane (simple exclusion)
    INTERSECT_TYPE intersect(const Frustum& f, const Sphere& s, const mat4x4* mat = nullptr);
    INTERSECT_TYPE intersect(const Frustum& f, const AABB& aabb, const mat4x4* mat = nullptr);
    INTERSECT_TYPE intersect(const Frustum& f, const OBB& obb, const mat4x4* mat = nullptr);
    INTERSECT_TYPE intersect(const AABB* aabb1, const AABB& aabb2, const mat4x4* mat = nullptr);

    INTERSECT_TYPE intersect(const Frustum& f, const Primitive* primitive, const mat4x4* mat = nullptr);



    vec2 calcOrthogonalVector(vec2& v);
    void calcOrthogonalVectors(vec3& n, vec3& x, vec3& y);

    float calcVetexEdgeDistance(const vec3& v, const vec3& t, const vec3& h);
    float calcVertexEdgePoint(const vec3& v, const vec3& t, const vec3& h);
    void calcEdgeEdgeDistance(const vec3& t0, const vec3& h0, const vec3& t1, const vec3& h1, float& l0, float& l1);


    template<class T>
    std::shared_ptr<Primitive> get_best_primitive(T& functor, unsigned int size)
    {
        std::shared_ptr<AABB> aabb(new AABB(functor, size));
        //	std::shared_ptr<Sphere> sphere(new Sphere(functor, size));
        return aabb;
    }

    /*

    #define EXTERN_EXPORT(T)          \
        extern template boost::serialization::guid_defined< T >; \
        extern template const char * boost::serialization::guid< T >();


    EXTERN_EXPORT(Sphere)
    EXTERN_EXPORT(AABB)
    //BOOST_CLASS_EXPORT(Frustum)
    EXTERN_EXPORT(Primitive)
    */




    namespace boost
    {
        namespace serialization
        {

            template<class Archive>
            void serialize(Archive& ar, Sphere& s, const unsigned int)
            {
                ar& NVP(s.pos)& NVP(s.radius)& NVP(boost::serialization::base_object<Primitive>(s));
            }
            template<class Archive>
            void serialize(Archive& ar, AABB& s, const unsigned int)
            {
                ar& NVP(s.min)& NVP(s.max)& NVP(boost::serialization::base_object<Primitive>(s));
            }
            template<class Archive>
            void serialize(Archive&, Primitive&, const unsigned int)
            {
            }

            template<class Archive>
            void serialize(Archive& ar, std::_Big_uint128& a, const unsigned int)
            {
                ar& NVP(a._Upper);
                ar& NVP(a._Lower);
            }
        } // namespace serialization
    } // namespace boost


}


module :private;

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

// Plane block
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

void Frustum::normalize()
{
    for (int i = 0; i < planes::COUNT; ++i)
        p[i].normalize();
}

void Frustum::calculate(const mat4x4& mat)
{
    inverse_mat = mat;
    inverse_mat.inverse();

    vec3 local_points[] =
    {
        { -1, -1, 0 },	//0
        { 1, -1, 0 },	//1
        { -1, 1, 0 },	//2
        { 1, 1, 0 },	//3

        { -1, -1, 1 },	//4
        { 1, -1, 1 },	//5
        { -1, 1, 1 },	//6
        { 1, 1, 1 }		//7
    };

    //      6---7
    //     /|  / |
    //	  / 4---5
    //	 / / / /
    //  2---3 /
    //	|/	|/
    //  0---1
    for (unsigned int i = 0; i < 8; i++)
    {
        vec4 t = vec4(local_points[i].x, local_points[i].y, local_points[i].z, 1) * mat;
        global_points[i] = vec3(t.xyz) / t.w;
    }

    p[0] = Plane(global_points[0], global_points[1], global_points[2]);
    p[1] = Plane(global_points[6], global_points[7], global_points[5]);
    p[2] = Plane(global_points[2], global_points[6], global_points[4]);
    p[3] = Plane(global_points[7], global_points[3], global_points[5]);
    p[4] = Plane(global_points[2], global_points[3], global_points[6]);
    p[5] = Plane(global_points[1], global_points[0], global_points[4]);
}

// Intersection
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

INTERSECT_TYPE intersect(const Frustum& f, const OBB& obb, const mat4x4*)
{
    vec3 p[8];
    vec3 dx = obb.dx * obb.ox, dy = obb.dy * obb.oy, dz = obb.dz * obb.oz;
    p[0] = obb.pos;
    p[1] = obb.pos + dx;
    p[2] = obb.pos + dy;
    p[3] = obb.pos + dx + dy;
    p[4] = obb.pos + dz;
    p[5] = obb.pos + dx + dz;
    p[6] = obb.pos + dy + dz;
    p[7] = obb.pos + dx + dy + dz;

    for (int i = 0; i < Frustum::planes::COUNT; ++i)
    {
        bool bFullyOut = true;

        for (int j = 0; j < 8; ++j)
        {
            float d = f.GetFrustumPlane(Frustum::planes(i))(p[j]);

            if (d < 0)
            {
                bFullyOut = false;
                break;
            }
        }

        if (bFullyOut) return INTERSECT_TYPE::FULL_OUT;
    }

    return INTERSECT_TYPE::FULL_IN;
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

// Vector functions
vec2 calcOrthogonalVector(vec2& v)
{
    return vec2(-v.y, v.x);
}

void calcOrthogonalVectors(vec3& n, vec3& x, vec3& y)
{
    vec3 temp(1, 0, 0);

    if (1 - fabs(vec3::dot(n, temp)) < Math::eps6) temp = vec3(0, 1, 0);

    x = vec3::cross(n, temp).normalize();
    y = vec3::cross(n, x).normalize();
}

// Distance functions
float getVEdistance(const vec3& v, const vec3& t, const vec3& h)
{
    vec3 n = h - t;
    float dn = n.length_squared();

    if (dn < Math::eps2) return (v - t).length();

    n /= dn;
    float l = vec3::dot(n, v - t);

    if (l < 0) return (v - t).length();

    if (l > 1) return (v - h).length();

    n = t + l * (h - t);
    return (v - n).length();
}

float getVEpoint(const vec3& v, const vec3& t, const vec3& h)
{
    vec3 n = h - t;
    float dn = n.length_squared();

    if (dn < Math::eps2) return 0;

    n /= dn;
    float l = vec3::dot(n, v - t);
    return Math::saturate(l);
}

void getEEdistance(const vec3& t0, const vec3& h0, const vec3& t1, const vec3& h1, float& l0, float& l1)
{
    vec3 n0 = h0 - t0;
    vec3 n1 = h1 - t1;
    vec3 w = t0 - t1;
    float a = vec3::dot(n0, n0);
    float b = vec3::dot(n0, n1);
    float c = vec3::dot(n1, n1);
    float d = -vec3::dot(w, n0);
    float e = -vec3::dot(w, n1);
    float D = b * b - a * c;
    float invD = 1.0f / D;
    l0 = (e * b - d * c) * invD;
    l1 = (a * e - b * d) * invD;

    // check regions
    if (l0 < 0 || l0 > 1 || l1 < 0 || l1 > 1 || fabs(D) < Math::eps2)
    {
        vec3 p;
        float minDist, tl0, tl1, tDist;
        // case1
        l0 = 0;
        l1 = getVEpoint(t0, t1, h1);
        p = t1 + l1 * n1;
        minDist = (t0 - p).length_squared();
        // case2
        tl0 = 1;
        tl1 = getVEpoint(h0, t1, h1);
        p = t1 + tl1 * n1;
        tDist = (h0 - p).length_squared();

        if (tDist < minDist) { minDist = tDist; l0 = tl0; l1 = tl1; }

        // case3
        tl1 = 0;
        tl0 = getVEpoint(t1, t0, h0);
        p = t0 + tl0 * n0;
        tDist = (t1 - p).length_squared();

        if (tDist < minDist) { minDist = tDist; l0 = tl0; l1 = tl1; }

        // case4
        tl1 = 1;
        tl0 = getVEpoint(h1, t0, h0);
        p = t0 + tl0 * n0;
        tDist = (h1 - p).length_squared();

        if (tDist < minDist) { minDist = tDist; l0 = tl0; l1 = tl1; }
    }
}



/*
*/



BOOST_CLASS_EXPORT_GUID(Sphere, "Sphere")
BOOST_CLASS_EXPORT_GUID(AABB, "AABB")
BOOST_CLASS_EXPORT_GUID(Primitive, "Primitive")

#pragma once
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
		virtual void apply_transform(ptr, mat4x4 & ) {};
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

        Ray(vec3 p, vec3 r): pos(p), dir(r)
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

		void apply_transform(Primitive::ptr p, mat4x4 & m) override{
		
			pos = static_cast<Sphere*>(p.get())->pos*m;
		//scale is not supported now
		};
};

class AABB: public Primitive
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
            min =  other->get_min();
            max =  other->get_max();
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


	void apply_transform(Primitive::ptr r, mat4x4 & mi) override {

			auto &aabb = *static_cast<AABB*>(r.get());

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
INTERSECT_TYPE intersect(const Frustum& f, const Sphere& s, const mat4x4* mat=nullptr);
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
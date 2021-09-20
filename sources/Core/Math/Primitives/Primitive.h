#pragma once
#include "../Constants.h"
#include "../Types/Vectors.h"
#include "../Types/Matrices.h"

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
	
        virtual const primitive_types get_type() const;

		virtual float get_volume() const;

		virtual void combine(Primitive*);
		virtual void combine(Primitive*, mat4x4&);
		virtual void apply_transform(ptr, mat4x4&);;
        virtual void set(Primitive*, mat4x4&);

		virtual vec3 get_min() = 0;
        virtual vec3 get_max() = 0;
        virtual ~Primitive() = default;
        virtual ptr clone() = 0;

private:
    SERIALIZE() {}
};



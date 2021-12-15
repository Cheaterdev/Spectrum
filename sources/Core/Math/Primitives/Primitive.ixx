module;
#include "Serialization/serialization_defines.h"
#include "Serialization/serialization_archives.h"
export module Primitive;

export import Constants;
export import Vectors;
export import Quaternion;
export import Matrices;
import stl.memory;
import serialization;

export enum class primitive_types
{
    sphere,
    aabb,
    frustum,
    unknown
};
export class Primitive
{
    public:
		using ptr = std::shared_ptr<Primitive>;
	
        virtual const primitive_types get_type() const;

		virtual float get_volume() const;

		virtual void combine(Primitive*);
		virtual void combine(Primitive*, mat4x4&);
		virtual void apply_transform(ptr, mat4x4&);;
        virtual void set(Primitive*, mat4x4&);

        virtual vec3 get_min() { return 0; }
        virtual vec3 get_max() { return 0; }
        virtual ~Primitive() = default;
        virtual ptr clone() { return nullptr; }

private:
    SERIALIZE() {}
};

module: private;


const primitive_types Primitive::get_type() const
{
    return primitive_types::unknown;
}

float Primitive::get_volume() const
{
    return 0;
}

void Primitive::combine(Primitive*)
{}

void Primitive::combine(Primitive*, mat4x4&)
{}

void Primitive::apply_transform(ptr, mat4x4&)
{}

void Primitive::set(Primitive*, mat4x4&)
{}


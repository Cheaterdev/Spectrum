export module Sphere;

export import Constants;
export import Vectors;
export import Quaternion;
export import Matrices;
export import Primitive;

import stl.memory;
import serialization;

export class Sphere : public Primitive
{
public:
	std::shared_ptr<Primitive>clone() override
	{
		return std::shared_ptr<Primitive>(new Sphere(*this));
	};

	const primitive_types get_type() const override
	{
		return primitive_types::sphere;
	}

	Sphere() = default;
	Sphere(vec3 pos, float radius);

	inline float get_volume() const override
	{
		const float cfVolume = 4.0f / 3.0f * Math::pi;
		return cfVolume * radius * radius * radius;
	}

	float GetSurfaceArea() const;

	vec3 pos;
	float radius;

	inline vec3 get_min() override
	{
		return pos - vec3(radius, radius, radius);
	}

	inline vec3 get_max() override
	{
		return pos + vec3(radius, radius, radius);
	}

	void apply_transform(Primitive::ptr p, mat4x4& m) override {

		pos = static_cast<Sphere*>(p.get())->pos * m;
		//scale is not supported now
	};

private:
	SERIALIZE()
	{
		SAVE_PARENT(Primitive);
		ar& NVP(pos)& NVP(radius);
	}
};



module: private;

REGISTER_TYPE(Sphere)

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


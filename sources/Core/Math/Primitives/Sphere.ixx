export module Core:Math.Sphere;

import :Math.Constants;
import :Math.Vectors;
import :Math.Quaternion;
import :Math.Matrices;
import :Math.Primitive;

import stl.memory;
import :serialization;

export class Sphere : public Primitive
{
public:
	std::shared_ptr<Primitive> clone() override;

	const primitive_types get_type() const override;

	Sphere() = default;
	Sphere(vec3 pos, float radius);

	float get_volume() const override;

	float GetSurfaceArea() const;

	vec3 pos;
	float radius;

	vec3 get_min() override;

	vec3 get_max() override;

	void apply_transform(Primitive::ptr p, mat4x4& m) override;

private:
	SERIALIZE()
	{
		SAVE_PARENT(Primitive);
		ar& NVP(pos)& NVP(radius);
	}
};



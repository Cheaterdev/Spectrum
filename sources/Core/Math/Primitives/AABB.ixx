export module Core:Math.AABB;

import :Math.Constants;
import :Math.Vectors;
import :Math.Quaternion;
import :Math.Matrices;
import :Math.Primitive;

import stl.memory;
import stl.core;
import :serialization;

export
{
	class AABB : public Primitive
	{
	public:
		vec3 min, max;

		AABB() = default;
		AABB(Primitive* other);
		AABB(std::function<vec3(uint)> functor, unsigned int size);

		void set(Primitive* other);

		std::shared_ptr<Primitive> clone() override;
		const primitive_types get_type() const override;
		float get_volume() const override;
		void set(Primitive* other, mat4x4& m) override;

		vec3 get_min() override;
		vec3 get_max() override;
		void combine(Primitive* other) override;
		void combine(Primitive* other, mat4x4& m) override;
		void apply_transform(ptr r, mat4x4& mi) override;
	private:
		SERIALIZE()
		{
			 SAVE_PARENT(Primitive);
			ar& NVP(min)& NVP(max);
		}
	};
}


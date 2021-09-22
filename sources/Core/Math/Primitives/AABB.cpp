#include "pch.h"
#include "AABB.h"

BOOST_CLASS_EXPORT(AABB)


AABB::AABB(Primitive* other) :min(other->get_min()), max(other->get_max())
{
}

AABB::AABB(std::function<vec3(UINT)> functor, unsigned size) : min(functor(0)), max(min)
{
	for (unsigned int i = 1; i < size; i++)
	{
		vec3 pos = functor(i);
		min = vec3::min(min, pos);
		max = vec3::max(max, pos);
	}
}

const primitive_types AABB::get_type() const
{
	return primitive_types::aabb;
}

float AABB::get_volume() const
{
	vec3 size = max - min;
	return size.x * size.y * size.z;
}


void AABB::set(Primitive* other)
{
	min = other->get_min();
	max = other->get_max();
}

void AABB::set(Primitive* other, mat4x4& m)
{
	vec3 a = other->get_min() * m;
	vec3 b = other->get_max() * m;
	min = vec3::min(a, b);
	max = vec3::max(a, b);
}

vec3 AABB::get_min()
{
	return min;
}

vec3 AABB::get_max()
{
	return max;
}

void AABB::combine(Primitive* other)
{
	min = vec3::min(min, other->get_min());
	max = vec3::max(max, other->get_max());
}

void AABB::combine(Primitive* other, mat4x4& m)
{
	min = vec3::min(min, other->get_min() * m);
	max = vec3::max(max, other->get_max() * m);
}

void AABB::apply_transform(ptr primitive, mat4x4& mi)
{
	auto& aabb = *static_cast<AABB*>(primitive.get());

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
}

std::shared_ptr<Primitive> AABB::clone()
{
	return std::make_shared<AABB>(*this);
};

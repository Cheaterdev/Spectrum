#include "pch.h"
#include "Primitive.h"

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


BOOST_CLASS_EXPORT(Primitive)

module Core:Math.Primitive;

import :Math.Vectors;
import :Math.Matrices;

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



REGISTER_TYPE(Primitive)
CEREAL_FORCE_REGISTER(Primitive);
module Core:Math.Sphere;

import :Math.Constants;
import :Math.Vectors;
import :Math.Matrices;
import :Math.Primitive;

std::shared_ptr<Primitive> Sphere::clone()
{
	return std::shared_ptr<Primitive>(new Sphere(*this));
}

const primitive_types Sphere::get_type() const
{
	return primitive_types::sphere;
}

float Sphere::get_volume() const
{
	const float cfVolume = 4.0f / 3.0f * Math::pi;
	return cfVolume * radius * radius * radius;
}

vec3 Sphere::get_min()
{
	return pos - vec3(radius, radius, radius);
}

vec3 Sphere::get_max()
{
	return pos + vec3(radius, radius, radius);
}

void Sphere::apply_transform(Primitive::ptr p, mat4x4& m)
{
	pos = static_cast<Sphere*>(p.get())->pos * m;
	//scale is not supported now
}

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

REGISTER_TYPE(Sphere)
CEREAL_FORCE_REGISTER(Sphere);

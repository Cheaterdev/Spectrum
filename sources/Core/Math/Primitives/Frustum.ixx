module;
#include "Serialization/serialization_defines.h"
export module Frustum;

export import Constants;
export import Vectors;
export import Quaternion;
export import Matrices;
export import Primitive;
import Plane;

import stl.memory;
import serialization;

export class Frustum : public Primitive
{
    vec3 global_points[8];
    mat4x4 inverse_mat;
public:
    std::shared_ptr<Primitive>clone() override;
    mat4x4 get_transform() const;

    vec3 get_max() override;
    vec3 get_min() override;
    const primitive_types get_type() const override;
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

    Plane& GetFrustumPlane(planes plane);
    const Plane& GetFrustumPlane(planes plane) const;
protected:
    Plane p[planes::COUNT];

private:
    SERIALIZE()
    {
        ar& SAVE_PARENT(Primitive);
        ar& NVP(global_points)& NVP(inverse_mat)& NVP(p);
    }
};



module: private;


vec3 Frustum::get_min()
{
	vec3 min = global_points[0];

	for (auto& point : global_points)
		min = vec3::min(min, point);

	return min;
}

mat4x4 Frustum::get_transform() const
{
	return inverse_mat;
}

vec3 Frustum::get_max()
{
	vec3 max = global_points[0];

	for (auto& point : global_points)
		max = vec3::max(max, point);

	return max;
}

const primitive_types Frustum::get_type() const
{
	return primitive_types::frustum;
}

Plane& Frustum::GetFrustumPlane(planes plane)
{
	return p[plane];
}

const Plane& Frustum::GetFrustumPlane(planes plane) const
{
	return p[plane];
}

void Frustum::normalize()
{
	for (int i = 0; i < planes::COUNT; ++i)
		p[i].normalize();
}

void Frustum::calculate(const mat4x4& mat)
{
	inverse_mat = mat;
	inverse_mat.inverse();

	vec3 local_points[] =
	{
		{ -1, -1, 0 },	//0
		{ 1, -1, 0 },	//1
		{ -1, 1, 0 },	//2
		{ 1, 1, 0 },	//3

		{ -1, -1, 1 },	//4
		{ 1, -1, 1 },	//5
		{ -1, 1, 1 },	//6
		{ 1, 1, 1 }		//7
	};

	//      6---7
	//     /|  / |
	//	  / 4---5
	//	 / / / /
	//  2---3 /
	//	|/	|/
	//  0---1
	for (unsigned int i = 0; i < 8; i++)
	{
		vec4 t = vec4(local_points[i].x, local_points[i].y, local_points[i].z, 1) * mat;
		global_points[i] = vec3(t.xyz) / t.w;
	}

	p[0] = Plane(global_points[0], global_points[1], global_points[2]);
	p[1] = Plane(global_points[6], global_points[7], global_points[5]);
	p[2] = Plane(global_points[2], global_points[6], global_points[4]);
	p[3] = Plane(global_points[7], global_points[3], global_points[5]);
	p[4] = Plane(global_points[2], global_points[3], global_points[6]);
	p[5] = Plane(global_points[1], global_points[0], global_points[4]);
}

std::shared_ptr<Primitive> Frustum::clone()
{
	return std::shared_ptr<Primitive>(new Frustum(*this));
};

BOOST_CLASS_EXPORT(Frustum)


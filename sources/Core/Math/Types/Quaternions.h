#pragma once

#include "Vectors.h"
class quat : public vec4
{
public:
	quat() = default;
	quat(float x, float y, float z, float w = 1);
	quat(vec3 dir, float angle);
	quat(vec3 v);
	quat(vec4 v);

	float norm() const;
	float magnitude() const;

	quat& conjugate();
	quat& inverse();

	void identity();
	quat operator*(const quat& q) const;
	quat& operator*=(const quat& q);

	void to_axis_angle(vec3 &axis, float &angle) const;
	vec3 rotate(const vec3 &vec);
};
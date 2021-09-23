#include "pch.h"
#include "Quaternions.h"

quat::quat(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

quat::quat(vec3 dir, float angle)
{
	dir.normalize();
	angle *= 0.5f;
	w = cos(angle);
	dir *= sin(angle);
	x = dir.x;
	y = dir.y;
	z = dir.z;
}

quat::quat(vec3 v) : vec4(v.x, v.y, v.z, 0)
{}

quat::quat(vec4 v) : vec4(v.x, v.y, v.z, v.w)
{}

float quat::norm() const
{ return x*x + y*y + z*z + w*w; }

float quat::magnitude() const
{ return sqrt(norm()); }

quat& quat::conjugate()
{ x *= -1; y *= -1; z *= -1; return *this; }

quat& quat::inverse()
{
	conjugate();

	return *this;
}

void quat::identity()
{ x = y = z = 0; w = 1; }

void quat::to_axis_angle(vec3& axis, float& angle) const
{

	angle = acosf(w);

	// pre-compute to save time
	float sinf_theta_inv = 1.0f / sinf(angle);

	// now the vector
	axis.x = x * sinf_theta_inv;
	axis.y = y * sinf_theta_inv;
	axis.z = z * sinf_theta_inv;

	// multiply by 2
	angle *= 2;
}

vec3 quat::rotate(const vec3& vec)
{
	float num = x * 2.f;
	float num2 = y * 2.f;
	float num3 = z * 2.f;
	float num4 = x * num;
	float num5 = y * num2;
	float num6 = z * num3;
	float num7 = x * num2;
	float num8 = x * num3;
	float num9 = y * num3;
	float num10 =w * num;
	float num11 = w * num2;
	float num12 = w * num3;

	float tx = (1.f - (num5 + num6)) * vec.x + (num7 + num12) * vec.y + (num8 - num11) * vec.z;
	float ty = (num7 - num12) * vec.x + (1.f - (num4 + num6)) * vec.y + (num9 + num10) * vec.z;
	float tz = (num8 + num11) * vec.x + (num9 - num10) * vec.y + (1.f - (num4 + num5)) * vec.z;
	//vec3 t;
	//t.set(tx, ty, tz);
	return { tx, ty, tz };
}

quat quat::operator*(const quat& q) const
{
	vec3 v1(x, y, z), v2(q.x, q.y, q.z), temp = vec3::cross(v1, v2) + q.w*v1 + w*v2;
	quat p(temp);
	p.w = w*q.w - dot(v1, v2);
	return p;
}

quat& quat::operator*=(const quat& q)
{
	vec3 v1(x, y, z), v2(q.x, q.y, q.z), temp = vec3::cross(v1, v2) + q.w*v1 + w*v2;
	x = temp.x;
	y = temp.y;
	z = temp.z;
	w = w*q.w - dot(v1, v2);
	return *this;
}

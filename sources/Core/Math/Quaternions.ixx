#include <cmath>

export module Quaternions;


import Vectors;
import Constants;


//class mat4x4;
export class quat : public vec4
{
public:
	quat() {}
	quat(float x, float y, float z, float w = 1);
	quat(vec3 dir, float angle);
	quat(vec3 v);
	quat(vec4 v);

	float norm() const { return x * x + y * y + z * z + w * w; }
	float magnitude() const { return sqrt(norm()); }
	quat& conjugate() { x *= -1; y *= -1; z *= -1; return *this; }
	quat& inverse()
	{
		conjugate();

		return *this;
	}

	void identity() { x = y = z = 0; w = 1; }
	quat operator*(const quat& q) const;
	quat& operator*=(const quat& q);
	//	void toMatrix(mat4x4& m) const;

	void to_axis_angle(vec3& axis, float& angle) const
	{

		angle = std::acosf(w);

		// pre-compute to save time
		float sinf_theta_inv = 1.0f / sinf(angle);

		// now the vector
		axis.x = x * sinf_theta_inv;
		axis.y = y * sinf_theta_inv;
		axis.z = z * sinf_theta_inv;

		// multiply by 2
		angle *= 2;
	}

	//! rotates v by this quat (quat must be unit)
	vec3 rotate(const vec3& vec)
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
		float num10 = w * num;
		float num11 = w * num2;
		float num12 = w * num3;

		float tx = (1.f - (num5 + num6)) * vec.x + (num7 + num12) * vec.y + (num8 - num11) * vec.z;
		float ty = (num7 - num12) * vec.x + (1.f - (num4 + num6)) * vec.y + (num9 + num10) * vec.z;
		float tz = (num8 + num11) * vec.x + (num9 - num10) * vec.y + (1.f - (num4 + num5)) * vec.z;
		//vec3 t;
		//t.set(tx, ty, tz);
		return { tx, ty, tz };
	}


};



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

quat quat::operator*(const quat& q) const
{
	vec3 v1(x, y, z), v2(q.x, q.y, q.z), temp = cross(v1, v2) + q.w * v1 + w * v2;
	quat p(temp);
	p.w = w * q.w - dot(v1, v2);
	return p;
}

quat& quat::operator*=(const quat& q)
{
	vec3 v1(x, y, z), v2(q.x, q.y, q.z), temp = cross(v1, v2) + q.w * v1 + w * v2;
	x = temp.x;
	y = temp.y;
	z = temp.z;
	w = w * q.w - dot(v1, v2);
	return *this;
}

/*
void quat::toMatrix(mat4x4& m) const
{
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
	float s = 2 / norm();
	x2 = x*s;	y2 = y*s;	z2 = z*s;
	xx = x*x2;	xy = x*y2;	xz = x*z2;
	yy = y*y2;	yz = y*z2;	zz = z*z2;
	wx = w*x2;	wy = w*y2;	wz = w*z2;

	m.a11 = 1 - (yy + zz);
	m.a12 = xy - wz;
	m.a13 = xz + wy;
	m.a14 = 0;

	m.a21 = xy + wz;
	m.a22 = 1 - (xx + zz);
	m.a23= yz - wx;
	m.a24 = 0;

	m.a31 = xz - wy;
	m.a32 = yz + wx;
	m.a33 = 1 - (xx + yy);
	m.a34 = 0;

	m.a41 = 0;
	m.a42 = 0;
	m.a43 = 0;
	m.a44 = 1;
}*/
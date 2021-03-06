#include "pch.h"

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
	vec3 v1(x, y, z), v2(q.x, q.y, q.z), temp = cross(v1, v2) + q.w*v1 + w*v2;
	quat p(temp);
	p.w = w*q.w - dot(v1, v2);
	return p;
}

quat& quat::operator*=(const quat& q)
{
	vec3 v1(x, y, z), v2(q.x, q.y, q.z), temp = cross(v1, v2) + q.w*v1 + w*v2;
	x = temp.x;
	y = temp.y;
	z = temp.z;
	w = w*q.w - dot(v1, v2);
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
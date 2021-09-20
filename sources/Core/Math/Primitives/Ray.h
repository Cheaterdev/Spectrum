#pragma once
#include "Primitive.h"


class Ray
{
public:

	vec3 pos;
	vec3 dir;

	Ray(vec3 p, vec3 r);

	bool intersect(Plane& p, vec3& res);

private:
	Ray();
	SERIALIZE()
	{
		ar& NVP(pos)& NVP(dir);
	}
};

inline Ray::Ray(vec3 p, vec3 r): pos(p), dir(r)
{
}

inline bool Ray::intersect(Plane& p, vec3& res)
{
	float denom = vec3::dot(p.n, dir); // dot(planeNormal, getDirection());

	if (denom != 0.0f)
	{
		float dist = vec3::dot(p.n, p.origin - pos) / denom;
		res = pos + dist * dir;
		return true;
	}

	return false;
}

inline Ray::Ray() = default;

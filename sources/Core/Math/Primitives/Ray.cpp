module Core:Math.Ray;

import :Math.Vectors;
import :Math.Plane;

Ray::Ray(vec3 p, vec3 r) : pos(p), dir(r)
{
}

bool Ray::intersect(Plane& p, vec3& res)
{
	float denom = vec3::dot(p.n, dir);

	if (denom != 0.0f)
	{
		float dist = vec3::dot(p.n, p.origin - pos) / denom;
		res = pos + dist * dir;
		return true;
	}

	return false;
}

Ray::Ray() = default;

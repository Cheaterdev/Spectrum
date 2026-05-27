export module Core:Math.Ray;

import :Math.Constants;
import :Math.Vectors;
import :Math.Quaternion;
import :Math.Matrices;
import :Math.Primitive;

import :Math.Plane;

import stl.memory;
import :serialization;

export class Ray
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




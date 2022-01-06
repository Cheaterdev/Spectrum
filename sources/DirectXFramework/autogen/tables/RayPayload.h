#pragma once
#include "RayCone.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct RayPayload
	{
		float4 color;
		float3 dir;
		uint recursion;
		float dist;
		RayCone cone;
		float4& GetColor() { return color; }
		float3& GetDir() { return dir; }
		uint& GetRecursion() { return recursion; }
		float& GetDist() { return dist; }
		RayCone& MapCone() { return cone; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(color);
			compiler.compile(dir);
			compiler.compile(recursion);
			compiler.compile(dist);
			compiler.compile(cone);
		}
	};
	#pragma pack(pop)
}

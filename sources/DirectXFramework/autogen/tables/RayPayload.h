#pragma once
#include "RayCone.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct RayPayload
	{
		struct CB
		{
			float4 color;
			float3 dir;
			uint recursion;
			float dist;
			RayCone::CB cone;
		} &cb;
		float4& GetColor() { return cb.color; }
		float3& GetDir() { return cb.dir; }
		uint& GetRecursion() { return cb.recursion; }
		float& GetDist() { return cb.dist; }
		RayCone MapCone() { return RayCone(cb.cone); }
		RayPayload(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}

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
			uint recursion;
			float dist;
			RayCone::CB cone;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		float4& GetColor() { return cb.color; }
		uint& GetRecursion() { return cb.recursion; }
		float& GetDist() { return cb.dist; }
		RayCone MapCone() { return RayCone(cb.cone); }
		RayPayload(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}

#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct AABB
	{
		struct CB
		{
			float4 min;
			float4 max;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		float4& GetMin() { return cb.min; }
		float4& GetMax() { return cb.max; }
		AABB(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}

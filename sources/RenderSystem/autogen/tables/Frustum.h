#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Frustum
	{
		struct CB
		{
			float4 planes[6];
		} &cb;
		float4* GetPlanes() { return cb.planes; }
		Frustum(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}

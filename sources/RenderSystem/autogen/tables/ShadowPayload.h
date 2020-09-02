#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct ShadowPayload
	{
		struct CB
		{
			bool hit;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		bool& GetHit() { return cb.hit; }
		ShadowPayload(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}

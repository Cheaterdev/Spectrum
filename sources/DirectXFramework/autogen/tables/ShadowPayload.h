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
		bool& GetHit() { return cb.hit; }
		ShadowPayload(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}

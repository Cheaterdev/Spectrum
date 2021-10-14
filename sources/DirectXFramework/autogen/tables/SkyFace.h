#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SkyFace
	{
		struct CB
		{
			uint face;
		} &cb;
		uint& GetFace() { return cb.face; }
		SkyFace(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}

#pragma once
namespace Table 
{
	struct SkyFace
	{
		struct CB
		{
			uint face;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		uint& GetFace() { return cb.face; }
		SkyFace(CB&cb) :cb(cb){}
	};
}

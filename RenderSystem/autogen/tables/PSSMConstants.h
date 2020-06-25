#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct PSSMConstants
	{
		struct CB
		{
			int level;
			float time;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		int& GetLevel() { return cb.level; }
		float& GetTime() { return cb.time; }
		PSSMConstants(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}

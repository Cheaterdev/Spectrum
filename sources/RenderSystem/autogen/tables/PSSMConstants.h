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
		int& GetLevel() { return cb.level; }
		float& GetTime() { return cb.time; }
		PSSMConstants(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}

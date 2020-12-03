#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DebugStruct
	{
		struct CB
		{
			uint4 v;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		uint4& GetV() { return cb.v; }
		DebugStruct(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}

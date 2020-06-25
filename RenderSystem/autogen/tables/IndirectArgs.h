#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct IndirectArgs
	{
		struct CB
		{
			uint data[5];
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		uint* GetData() { return cb.data; }
		IndirectArgs(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}

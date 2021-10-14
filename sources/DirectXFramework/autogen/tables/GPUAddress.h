#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GPUAddress
	{
		struct CB
		{
			uint2 data;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		uint2& GetData() { return cb.data; }
		GPUAddress(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}

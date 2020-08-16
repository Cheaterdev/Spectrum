#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Instance
	{
		struct CB
		{
			uint instanceId;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		uint& GetInstanceId() { return cb.instanceId; }
		Instance(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}

#pragma once
namespace Table 
{
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
}

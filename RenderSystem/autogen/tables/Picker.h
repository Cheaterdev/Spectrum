#pragma once
namespace Table 
{
	struct Picker
	{
		struct CB
		{
			uint instanceId;
		} &cb;
		using SRV = Empty;
		struct UAV
		{
			Render::Handle viewBuffer;
		} &uav;
		using SMP = Empty;
		uint& GetInstanceId() { return cb.instanceId; }
		Render::Handle& GetViewBuffer() { return uav.viewBuffer; }
		Picker(CB&cb,UAV&uav) :cb(cb),uav(uav){}
	};
}

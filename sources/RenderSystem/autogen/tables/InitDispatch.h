#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct InitDispatch
	{
		using CB = Empty;
		using SRV = Empty;
		struct UAV
		{
			Render::Handle counter;
			Render::Handle dispatch_data;
		} &uav;
		using SMP = Empty;
		Render::Handle& GetCounter() { return uav.counter; }
		Render::Handle& GetDispatch_data() { return uav.dispatch_data; }
		InitDispatch(UAV&uav) :uav(uav){}
	};
	#pragma pack(pop)
}

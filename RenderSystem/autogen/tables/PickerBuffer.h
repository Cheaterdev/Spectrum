#pragma once
namespace Table 
{
	struct PickerBuffer
	{
		using CB = Empty;
		using SRV = Empty;
		struct UAV
		{
			Render::Handle viewBuffer;
		} &uav;
		using SMP = Empty;
		Render::Handle& GetViewBuffer() { return uav.viewBuffer; }
		PickerBuffer(UAV&uav) :uav(uav){}
	};
}

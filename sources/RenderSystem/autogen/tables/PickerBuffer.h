#pragma once
namespace Table 
{
	#pragma pack(push, 1)
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
	#pragma pack(pop)
}

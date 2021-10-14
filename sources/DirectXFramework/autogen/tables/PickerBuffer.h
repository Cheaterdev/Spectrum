#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct PickerBuffer
	{
		struct UAV
		{
			Render::HLSL::RWStructuredBuffer<uint> viewBuffer;
		} &uav;
		Render::HLSL::RWStructuredBuffer<uint>& GetViewBuffer() { return uav.viewBuffer; }
		PickerBuffer(UAV&uav) :uav(uav){}
	};
	#pragma pack(pop)
}

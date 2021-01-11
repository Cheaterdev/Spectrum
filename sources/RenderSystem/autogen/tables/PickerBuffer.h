#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct PickerBuffer
	{
		struct UAV
		{
			DX12::HLSL::RWStructuredBuffer<uint> viewBuffer;
		} &uav;
		DX12::HLSL::RWStructuredBuffer<uint>& GetViewBuffer() { return uav.viewBuffer; }
		PickerBuffer(UAV&uav) :uav(uav){}
	};
	#pragma pack(pop)
}

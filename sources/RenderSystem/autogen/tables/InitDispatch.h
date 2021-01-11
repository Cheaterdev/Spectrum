#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct InitDispatch
	{
		struct UAV
		{
			DX12::HLSL::RWStructuredBuffer<uint> counter;
			DX12::HLSL::RWStructuredBuffer<DispatchArguments> dispatch_data;
		} &uav;
		DX12::HLSL::RWStructuredBuffer<uint>& GetCounter() { return uav.counter; }
		DX12::HLSL::RWStructuredBuffer<DispatchArguments>& GetDispatch_data() { return uav.dispatch_data; }
		InitDispatch(UAV&uav) :uav(uav){}
	};
	#pragma pack(pop)
}

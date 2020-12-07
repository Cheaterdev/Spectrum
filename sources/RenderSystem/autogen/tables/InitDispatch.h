#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct InitDispatch
	{
		struct UAV
		{
			Render::HLSL::RWStructuredBuffer<uint> counter;
			Render::HLSL::RWStructuredBuffer<DispatchArguments> dispatch_data;
		} &uav;
		Render::HLSL::RWStructuredBuffer<uint>& GetCounter() { return uav.counter; }
		Render::HLSL::RWStructuredBuffer<DispatchArguments>& GetDispatch_data() { return uav.dispatch_data; }
		InitDispatch(UAV&uav) :uav(uav){}
	};
	#pragma pack(pop)
}

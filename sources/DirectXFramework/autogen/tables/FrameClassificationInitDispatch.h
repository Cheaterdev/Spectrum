#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct FrameClassificationInitDispatch
	{
		struct SRV
		{
			Render::HLSL::StructuredBuffer<uint> hi_counter;
			Render::HLSL::StructuredBuffer<uint> low_counter;
		} &srv;
		struct UAV
		{
			Render::HLSL::RWStructuredBuffer<DispatchArguments> hi_dispatch_data;
			Render::HLSL::RWStructuredBuffer<DispatchArguments> low_dispatch_data;
		} &uav;
		Render::HLSL::StructuredBuffer<uint>& GetHi_counter() { return srv.hi_counter; }
		Render::HLSL::StructuredBuffer<uint>& GetLow_counter() { return srv.low_counter; }
		Render::HLSL::RWStructuredBuffer<DispatchArguments>& GetHi_dispatch_data() { return uav.hi_dispatch_data; }
		Render::HLSL::RWStructuredBuffer<DispatchArguments>& GetLow_dispatch_data() { return uav.low_dispatch_data; }
		FrameClassificationInitDispatch(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}

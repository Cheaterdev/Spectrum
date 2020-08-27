#pragma once
#include "GBuffer.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct RaytracingRays
	{
		using CB = Empty;
		struct SRV
		{
			GBuffer::SRV gbuffer;
		} &srv;
		struct UAV
		{
			Render::Handle output;
			GBuffer::UAV gbuffer;
		} &uav;
		using SMP = Empty;
		Render::Handle& GetOutput() { return uav.output; }
		GBuffer MapGbuffer() { return GBuffer(srv.gbuffer); }
		RaytracingRays(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}

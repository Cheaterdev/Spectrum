#pragma once
#include "GBuffer.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct RaytracingRays
	{
		struct CB
		{
			float pixelAngle;
			GBuffer::CB gbuffer;
		} &cb;
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
		float& GetPixelAngle() { return cb.pixelAngle; }
		GBuffer MapGbuffer() { return GBuffer(srv.gbuffer); }
		RaytracingRays(CB&cb,SRV&srv,UAV&uav) :cb(cb),srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}

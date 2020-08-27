#pragma once
#include "GBuffer.h"
struct RaytracingRays_srv
{
	GBuffer_srv gbuffer;
};
struct RaytracingRays_uav
{
	RWTexture2D<float4> output;
};
struct RaytracingRays
{
	RaytracingRays_srv srv;
	RaytracingRays_uav uav;
	RWTexture2D<float4> GetOutput() { return uav.output; }
	GBuffer GetGbuffer() { return CreateGBuffer(srv.gbuffer); }

};
 const RaytracingRays CreateRaytracingRays(RaytracingRays_srv srv,RaytracingRays_uav uav)
{
	const RaytracingRays result = {srv,uav
	};
	return result;
}

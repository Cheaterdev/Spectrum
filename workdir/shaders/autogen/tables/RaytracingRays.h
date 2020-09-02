#pragma once
#include "GBuffer.h"
struct RaytracingRays_cb
{
	float pixelAngle;
};
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
	RaytracingRays_cb cb;
	RaytracingRays_srv srv;
	RaytracingRays_uav uav;
	RWTexture2D<float4> GetOutput() { return uav.output; }
	float GetPixelAngle() { return cb.pixelAngle; }
	GBuffer GetGbuffer() { return CreateGBuffer(srv.gbuffer); }

};
 const RaytracingRays CreateRaytracingRays(RaytracingRays_cb cb,RaytracingRays_srv srv,RaytracingRays_uav uav)
{
	const RaytracingRays result = {cb,srv,uav
	};
	return result;
}

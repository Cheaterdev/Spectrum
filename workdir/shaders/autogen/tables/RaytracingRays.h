#pragma once
#include "GBuffer.h"
struct RaytracingRays
{
	float pixelAngle; // float
	uint output; // RWTexture2D<float4>
	GBuffer gbuffer; // GBuffer
	GBuffer GetGbuffer() { return gbuffer; }
	float GetPixelAngle() { return pixelAngle; }
	RWTexture2D<float4> GetOutput() { return ResourceDescriptorHeap[output]; }
};

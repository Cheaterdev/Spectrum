#pragma once
#include "sig_hlsl.hlsl"
#include "GBuffer.h"
struct WorkGraphTest
{
	uint output; // RWTexture2D<float4>
	uint target; // RWTexture2D<float4>
	GBuffer gbuffer; // GBuffer
	GBuffer GetGbuffer() { return gbuffer; }
	RWTexture2D<float4> GetOutput() { return ResourceDescriptorHeap[output]; }
	RWTexture2D<float4> GetTarget() { return ResourceDescriptorHeap[target]; }
};
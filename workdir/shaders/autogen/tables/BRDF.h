#pragma once
#include "sig_hlsl.hlsl"
struct BRDF
{
	uint output; // RWTexture3D<float4>
	RWTexture3D<float4> GetOutput() { return ResourceDescriptorHeap[output]; }
};

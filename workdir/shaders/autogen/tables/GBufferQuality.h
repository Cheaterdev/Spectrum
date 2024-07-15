#pragma once
#include "sig_hlsl.hlsl"
struct GBufferQuality
{
	uint ref; // Texture2D<float4>
	Texture2D<float4> GetRef() { return ResourceDescriptorHeap[ref]; }
};
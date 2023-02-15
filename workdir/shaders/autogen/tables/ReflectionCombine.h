#pragma once
#include "GBuffer.h"
struct ReflectionCombine
{
	uint reflection; // Texture2D<float4>
	uint target; // RWTexture2D<float4>
	GBuffer gbuffer; // GBuffer
	GBuffer GetGbuffer() { return gbuffer; }
	Texture2D<float4> GetReflection() { return ResourceDescriptorHeap[reflection]; }
	RWTexture2D<float4> GetTarget() { return ResourceDescriptorHeap[target]; }
};

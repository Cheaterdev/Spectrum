#pragma once
#include "GBuffer.h"
struct PSSMLighting
{
	uint light_mask; // Texture2D<float>
	GBuffer gbuffer; // GBuffer
	GBuffer GetGbuffer() { return gbuffer; }
	Texture2D<float> GetLight_mask() { return ResourceDescriptorHeap[light_mask]; }
};

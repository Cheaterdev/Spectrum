#pragma once
#include "GBuffer.h"
struct VoxelDebug
{
	uint volume; // Texture3D<float4>
	GBuffer gbuffer; // GBuffer
	GBuffer GetGbuffer() { return gbuffer; }
	Texture3D<float4> GetVolume() { return ResourceDescriptorHeap[volume]; }
};

#pragma once
#include "VoxelTilingParams.h"
struct VoxelCopy
{
	uint Source[2]; // Texture3D<float4>
	uint Target[2]; // RWTexture3D<float4>
	VoxelTilingParams params; // VoxelTilingParams
	VoxelTilingParams GetParams() { return params; }
	RWTexture3D<float4> GetTarget(int i) { return ResourceDescriptorHeap[Target[i]]; }
	Texture3D<float4> GetSource(int i) { return ResourceDescriptorHeap[Source[i]]; }
};

#pragma once
#include "VoxelTilingParams.h"
struct VoxelMipMap
{
	uint SrcMip; // Texture3D<float4>
	uint OutMips[3]; // RWTexture3D<float4>
	VoxelTilingParams params; // VoxelTilingParams
	VoxelTilingParams GetParams() { return params; }
	RWTexture3D<float4> GetOutMips(int i) { return ResourceDescriptorHeap[OutMips[i]]; }
	Texture3D<float4> GetSrcMip() { return ResourceDescriptorHeap[SrcMip]; }
};

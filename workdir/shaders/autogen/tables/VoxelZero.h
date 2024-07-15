#pragma once
#include "sig_hlsl.hlsl"
#include "VoxelTilingParams.h"
struct VoxelZero
{
	uint Target; // RWTexture3D<float4>
	VoxelTilingParams params; // VoxelTilingParams
	VoxelTilingParams GetParams() { return params; }
	RWTexture3D<float4> GetTarget() { return ResourceDescriptorHeap[Target]; }
};
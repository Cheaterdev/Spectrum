#pragma once
#include "VoxelInfo.h"
struct Voxelization
{
	uint albedo; // RWTexture3D<float4>
	uint normals; // RWTexture3D<float4>
	uint visibility; // RWTexture3D<uint>
	VoxelInfo info; // VoxelInfo
	VoxelInfo GetInfo() { return info; }
	RWTexture3D<float4> GetAlbedo() { return ResourceDescriptorHeap[albedo]; }
	RWTexture3D<float4> GetNormals() { return ResourceDescriptorHeap[normals]; }
	RWTexture3D<uint> GetVisibility() { return ResourceDescriptorHeap[visibility]; }
};

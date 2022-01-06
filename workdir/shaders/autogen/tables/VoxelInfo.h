#pragma once
struct VoxelInfo
{
	float4 min; // float4
	float4 size; // float4
	uint4 voxels_per_tile; // uint4
	uint4 voxel_tiles_count; // uint4
	float4 GetMin() { return min; }
	float4 GetSize() { return size; }
	uint4 GetVoxels_per_tile() { return voxels_per_tile; }
	uint4 GetVoxel_tiles_count() { return voxel_tiles_count; }
};

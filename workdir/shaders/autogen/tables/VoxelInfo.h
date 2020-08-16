#pragma once
struct VoxelInfo_cb
{
	float4 min;
	float4 size;
	uint4 voxels_per_tile;
	uint4 voxel_tiles_count;
};
struct VoxelInfo
{
	VoxelInfo_cb cb;
	float4 GetMin() { return cb.min; }
	float4 GetSize() { return cb.size; }
	uint4 GetVoxels_per_tile() { return cb.voxels_per_tile; }
	uint4 GetVoxel_tiles_count() { return cb.voxel_tiles_count; }
};
 const VoxelInfo CreateVoxelInfo(VoxelInfo_cb cb)
{
	const VoxelInfo result = {cb
	};
	return result;
}

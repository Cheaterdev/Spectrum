#pragma once
struct VoxelTilingParams_cb
{
	uint4 voxels_per_tile; // uint4
};
struct VoxelTilingParams_srv
{
	uint tiles; // StructuredBuffer<int3>
};
struct VoxelTilingParams
{
	VoxelTilingParams_cb cb;
	VoxelTilingParams_srv srv;
	StructuredBuffer<int3> GetTiles() { return ResourceDescriptorHeap[srv.tiles]; }
	uint4 GetVoxels_per_tile() { return cb.voxels_per_tile; }

		
uint3 get_voxel_pos(uint3 dispatchID)
{
	uint tile_index = dispatchID.x / cb.voxels_per_tile.x;
	uint3 tile_pos = GetTiles()[tile_index] * cb.voxels_per_tile.xyz;

	uint3 tile_local_pos = dispatchID - int3(tile_index * cb.voxels_per_tile.x, 0, 0);
	uint3 index = tile_pos + tile_local_pos;
	return index;
}
		
	
};
 const VoxelTilingParams CreateVoxelTilingParams(VoxelTilingParams_cb cb,VoxelTilingParams_srv srv)
{
	const VoxelTilingParams result = {cb,srv
	};
	return result;
}

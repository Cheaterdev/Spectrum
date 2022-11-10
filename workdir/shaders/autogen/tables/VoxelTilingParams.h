#pragma once
struct VoxelTilingParams
{
	uint4 voxels_per_tile; // uint4
	uint tiles; // StructuredBuffer<int3>
	uint4 GetVoxels_per_tile() { return voxels_per_tile; }
	StructuredBuffer<int3> GetTiles() { return ResourceDescriptorHeap[tiles]; }
	
		
uint3 get_voxel_pos(uint3 dispatchID)
{
	uint tile_index = dispatchID.x / voxels_per_tile.x;
	uint3 tile_pos = GetTiles()[tile_index] * voxels_per_tile.xyz;

	uint3 tile_local_pos = dispatchID - int3(tile_index * voxels_per_tile.x, 0, 0);
	uint3 index = tile_pos + tile_local_pos;
	return index;
}
		
	
};

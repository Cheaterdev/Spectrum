#pragma once
struct TilingParams
{
	uint tiles; // StructuredBuffer<uint2>
	StructuredBuffer<uint2> GetTiles() { return ResourceDescriptorHeap[tiles]; }
	
		
uint2 get_pixel_pos(uint3 dispatchID)
{
	uint tile_index = dispatchID.x / 32;
	uint2 tile_pos = GetTiles()[tile_index] * 32;

	uint2 tile_local_pos = dispatchID.xy - int3(tile_index * 32, 0, 0);
	uint2 index = tile_pos + tile_local_pos;
	return index;
}
		
	
};

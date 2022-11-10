#pragma once
struct VoxelVisibility
{
	uint visibility; // Texture3D<uint>
	uint visible_tiles; // AppendStructuredBuffer<uint4>
	Texture3D<uint> GetVisibility() { return ResourceDescriptorHeap[visibility]; }
	AppendStructuredBuffer<uint4> GetVisible_tiles() { return ResourceDescriptorHeap[visible_tiles]; }
};

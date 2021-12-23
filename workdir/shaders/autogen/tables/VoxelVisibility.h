#pragma once
struct VoxelVisibility_srv
{
	uint visibility; // Texture3D<uint>
};
struct VoxelVisibility_uav
{
	uint visible_tiles; // AppendStructuredBuffer<uint4>
};
struct VoxelVisibility
{
	VoxelVisibility_srv srv;
	VoxelVisibility_uav uav;
	Texture3D<uint> GetVisibility() { return ResourceDescriptorHeap[srv.visibility]; }
	AppendStructuredBuffer<uint4> GetVisible_tiles() { return ResourceDescriptorHeap[uav.visible_tiles]; }

};
 const VoxelVisibility CreateVoxelVisibility(VoxelVisibility_srv srv,VoxelVisibility_uav uav)
{
	const VoxelVisibility result = {srv,uav
	};
	return result;
}

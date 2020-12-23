#pragma once
struct VoxelVisibility_srv
{
	Texture3D<uint> visibility;
};
struct VoxelVisibility_uav
{
	AppendStructuredBuffer<uint4> visible_tiles;
};
struct VoxelVisibility
{
	VoxelVisibility_srv srv;
	VoxelVisibility_uav uav;
	Texture3D<uint> GetVisibility() { return srv.visibility; }
	AppendStructuredBuffer<uint4> GetVisible_tiles() { return uav.visible_tiles; }

};
 const VoxelVisibility CreateVoxelVisibility(VoxelVisibility_srv srv,VoxelVisibility_uav uav)
{
	const VoxelVisibility result = {srv,uav
	};
	return result;
}

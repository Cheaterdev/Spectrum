#pragma once
struct VoxelZero_cb
{
	uint4 voxels_per_tile;
	uint groupCount;
};
struct VoxelZero_srv
{
	StructuredBuffer<int3> visibility;
};
struct VoxelZero_uav
{
	RWTexture3D<float4> Target;
};
struct VoxelZero
{
	VoxelZero_cb cb;
	VoxelZero_srv srv;
	VoxelZero_uav uav;
	uint4 GetVoxels_per_tile() { return cb.voxels_per_tile; }
	RWTexture3D<float4> GetTarget() { return uav.Target; }
	StructuredBuffer<int3> GetVisibility() { return srv.visibility; }
	uint GetGroupCount() { return cb.groupCount; }

};
 const VoxelZero CreateVoxelZero(VoxelZero_cb cb,VoxelZero_srv srv,VoxelZero_uav uav)
{
	const VoxelZero result = {cb,srv,uav
	};
	return result;
}

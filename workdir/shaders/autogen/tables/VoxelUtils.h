#pragma once
struct VoxelUtils_cb
{
	uint groupCount;
	uint4 voxels_per_tile;
};
struct VoxelUtils_srv
{
	Texture3D<float4> Source[2];
	StructuredBuffer<int3> visibility;
};
struct VoxelUtils_uav
{
	RWTexture3D<float4> Target[2];
};
struct VoxelUtils
{
	VoxelUtils_cb cb;
	VoxelUtils_srv srv;
	VoxelUtils_uav uav;
	RWTexture3D<float4> GetTarget(int i) { return uav.Target[i]; }
	Texture3D<float4> GetSource(int i) { return srv.Source[i]; }
	StructuredBuffer<int3> GetVisibility() { return srv.visibility; }
	uint GetGroupCount() { return cb.groupCount; }
	uint4 GetVoxels_per_tile() { return cb.voxels_per_tile; }

};
 const VoxelUtils CreateVoxelUtils(VoxelUtils_cb cb,VoxelUtils_srv srv,VoxelUtils_uav uav)
{
	const VoxelUtils result = {cb,srv,uav
	};
	return result;
}

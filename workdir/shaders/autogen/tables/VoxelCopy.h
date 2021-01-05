#pragma once
struct VoxelCopy_cb
{
	uint4 voxels_per_tile;
	uint groupCount;
};
struct VoxelCopy_srv
{
	Texture3D<float4> Source[2];
	StructuredBuffer<int3> visibility;
};
struct VoxelCopy_uav
{
	RWTexture3D<float4> Target[2];
};
struct VoxelCopy
{
	VoxelCopy_cb cb;
	VoxelCopy_srv srv;
	VoxelCopy_uav uav;
	uint4 GetVoxels_per_tile() { return cb.voxels_per_tile; }
	RWTexture3D<float4> GetTarget(int i) { return uav.Target[i]; }
	Texture3D<float4> GetSource(int i) { return srv.Source[i]; }
	StructuredBuffer<int3> GetVisibility() { return srv.visibility; }
	uint GetGroupCount() { return cb.groupCount; }

};
 const VoxelCopy CreateVoxelCopy(VoxelCopy_cb cb,VoxelCopy_srv srv,VoxelCopy_uav uav)
{
	const VoxelCopy result = {cb,srv,uav
	};
	return result;
}

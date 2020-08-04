#pragma once
struct VoxelMipMap_cb
{
	uint groupCount;
};
struct VoxelMipMap_srv
{
	Texture3D<float4> SrcMip;
	StructuredBuffer<int3> visibility;
};
struct VoxelMipMap_uav
{
	RWTexture3D<float4> OutMips[3];
};
struct VoxelMipMap
{
	VoxelMipMap_cb cb;
	VoxelMipMap_srv srv;
	VoxelMipMap_uav uav;
	RWTexture3D<float4> GetOutMips(int i) { return uav.OutMips[i]; }
	Texture3D<float4> GetSrcMip() { return srv.SrcMip; }
	StructuredBuffer<int3> GetVisibility() { return srv.visibility; }
	uint GetGroupCount() { return cb.groupCount; }
};
 const VoxelMipMap CreateVoxelMipMap(VoxelMipMap_cb cb,VoxelMipMap_srv srv,VoxelMipMap_uav uav)
{
	const VoxelMipMap result = {cb,srv,uav
	};
	return result;
}

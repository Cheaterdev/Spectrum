#pragma once
#include "VoxelInfo.h"
struct Voxelization_cb
{
	VoxelInfo_cb info;
};
struct Voxelization_uav
{
	RWTexture3D<float4> albedo;
	RWTexture3D<float4> normals;
	RWByteAddressBuffer visibility;
};
struct Voxelization
{
	Voxelization_cb cb;
	Voxelization_uav uav;
	RWTexture3D<float4> GetAlbedo() { return uav.albedo; }
	RWTexture3D<float4> GetNormals() { return uav.normals; }
	RWByteAddressBuffer GetVisibility() { return uav.visibility; }
	VoxelInfo GetInfo() { return CreateVoxelInfo(cb.info); }

};
 const Voxelization CreateVoxelization(Voxelization_cb cb,Voxelization_uav uav)
{
	const Voxelization result = {cb,uav
	};
	return result;
}

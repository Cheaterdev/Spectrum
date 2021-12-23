#pragma once
#include "VoxelInfo.h"
struct Voxelization_cb
{
	VoxelInfo_cb info;
};
struct Voxelization_uav
{
	uint albedo; // RWTexture3D<float4>
	uint normals; // RWTexture3D<float4>
	uint visibility; // RWTexture3D<uint>
};
struct Voxelization
{
	Voxelization_cb cb;
	Voxelization_uav uav;
	RWTexture3D<float4> GetAlbedo() { return ResourceDescriptorHeap[uav.albedo]; }
	RWTexture3D<float4> GetNormals() { return ResourceDescriptorHeap[uav.normals]; }
	RWTexture3D<uint> GetVisibility() { return ResourceDescriptorHeap[uav.visibility]; }
	VoxelInfo GetInfo() { return CreateVoxelInfo(cb.info); }

};
 const Voxelization CreateVoxelization(Voxelization_cb cb,Voxelization_uav uav)
{
	const Voxelization result = {cb,uav
	};
	return result;
}

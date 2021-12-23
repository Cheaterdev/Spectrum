#pragma once
#include "PSSMDataGlobal.h"
#include "VoxelTilingParams.h"
struct VoxelLighting_cb
{
	VoxelTilingParams_cb params;
};
struct VoxelLighting_srv
{
	uint albedo; // Texture3D<float4>
	uint normals; // Texture3D<float4>
	uint lower; // Texture3D<float4>
	uint tex_cube; // TextureCube<float4>
	PSSMDataGlobal_srv pssmGlobal;
	VoxelTilingParams_srv params;
};
struct VoxelLighting_uav
{
	uint output; // RWTexture3D<float4>
};
struct VoxelLighting
{
	VoxelLighting_cb cb;
	VoxelLighting_srv srv;
	VoxelLighting_uav uav;
	RWTexture3D<float4> GetOutput() { return ResourceDescriptorHeap[uav.output]; }
	Texture3D<float4> GetAlbedo() { return ResourceDescriptorHeap[srv.albedo]; }
	Texture3D<float4> GetNormals() { return ResourceDescriptorHeap[srv.normals]; }
	Texture3D<float4> GetLower() { return ResourceDescriptorHeap[srv.lower]; }
	TextureCube<float4> GetTex_cube() { return ResourceDescriptorHeap[srv.tex_cube]; }
	PSSMDataGlobal GetPssmGlobal() { return CreatePSSMDataGlobal(srv.pssmGlobal); }
	VoxelTilingParams GetParams() { return CreateVoxelTilingParams(cb.params,srv.params); }

};
 const VoxelLighting CreateVoxelLighting(VoxelLighting_cb cb,VoxelLighting_srv srv,VoxelLighting_uav uav)
{
	const VoxelLighting result = {cb,srv,uav
	};
	return result;
}

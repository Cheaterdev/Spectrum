#pragma once
#include "PSSMDataGlobal.h"
struct VoxelLighting_cb
{
	uint groupCount;
};
struct VoxelLighting_srv
{
	Texture3D<float4> albedo;
	Texture3D<float4> normals;
	Texture3D<float4> lower;
	TextureCube<float4> tex_cube;
	StructuredBuffer<int3> visibility;
	PSSMDataGlobal_srv pssmGlobal;
};
struct VoxelLighting_uav
{
	RWTexture3D<float4> output;
};
struct VoxelLighting
{
	VoxelLighting_cb cb;
	VoxelLighting_srv srv;
	VoxelLighting_uav uav;
	RWTexture3D<float4> GetOutput() { return uav.output; }
	Texture3D<float4> GetAlbedo() { return srv.albedo; }
	Texture3D<float4> GetNormals() { return srv.normals; }
	Texture3D<float4> GetLower() { return srv.lower; }
	TextureCube<float4> GetTex_cube() { return srv.tex_cube; }
	StructuredBuffer<int3> GetVisibility() { return srv.visibility; }
	uint GetGroupCount() { return cb.groupCount; }
	PSSMDataGlobal GetPssmGlobal() { return CreatePSSMDataGlobal(srv.pssmGlobal); }

};
 const VoxelLighting CreateVoxelLighting(VoxelLighting_cb cb,VoxelLighting_srv srv,VoxelLighting_uav uav)
{
	const VoxelLighting result = {cb,srv,uav
	};
	return result;
}

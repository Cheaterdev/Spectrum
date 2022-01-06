#pragma once
#include "PSSMDataGlobal.h"
#include "VoxelTilingParams.h"
struct VoxelLighting
{
	uint albedo; // Texture3D<float4>
	uint normals; // Texture3D<float4>
	uint lower; // Texture3D<float4>
	uint tex_cube; // TextureCube<float4>
	uint output; // RWTexture3D<float4>
	PSSMDataGlobal pssmGlobal; // PSSMDataGlobal
	VoxelTilingParams params; // VoxelTilingParams
	PSSMDataGlobal GetPssmGlobal() { return pssmGlobal; }
	VoxelTilingParams GetParams() { return params; }
	RWTexture3D<float4> GetOutput() { return ResourceDescriptorHeap[output]; }
	Texture3D<float4> GetAlbedo() { return ResourceDescriptorHeap[albedo]; }
	Texture3D<float4> GetNormals() { return ResourceDescriptorHeap[normals]; }
	Texture3D<float4> GetLower() { return ResourceDescriptorHeap[lower]; }
	TextureCube<float4> GetTex_cube() { return ResourceDescriptorHeap[tex_cube]; }
};

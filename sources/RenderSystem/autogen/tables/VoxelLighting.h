#pragma once
#include "PSSMDataGlobal.h"
#include "VoxelTilingParams.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelLighting
	{
		struct CB
		{
			VoxelTilingParams::CB params;
		} &cb;
		struct SRV
		{
			Render::HLSL::Texture3D<float4> albedo;
			Render::HLSL::Texture3D<float4> normals;
			Render::HLSL::Texture3D<float4> lower;
			Render::HLSL::TextureCube<float4> tex_cube;
			PSSMDataGlobal::SRV pssmGlobal;
			VoxelTilingParams::SRV params;
		} &srv;
		struct UAV
		{
			Render::HLSL::RWTexture3D<float4> output;
		} &uav;
		Render::HLSL::RWTexture3D<float4>& GetOutput() { return uav.output; }
		Render::HLSL::Texture3D<float4>& GetAlbedo() { return srv.albedo; }
		Render::HLSL::Texture3D<float4>& GetNormals() { return srv.normals; }
		Render::HLSL::Texture3D<float4>& GetLower() { return srv.lower; }
		Render::HLSL::TextureCube<float4>& GetTex_cube() { return srv.tex_cube; }
		PSSMDataGlobal MapPssmGlobal() { return PSSMDataGlobal(srv.pssmGlobal); }
		VoxelTilingParams MapParams() { return VoxelTilingParams(cb.params,srv.params); }
		VoxelLighting(CB&cb,SRV&srv,UAV&uav) :cb(cb),srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}

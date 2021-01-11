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
			DX12::HLSL::Texture3D<float4> albedo;
			DX12::HLSL::Texture3D<float4> normals;
			DX12::HLSL::Texture3D<float4> lower;
			DX12::HLSL::TextureCube<float4> tex_cube;
			PSSMDataGlobal::SRV pssmGlobal;
			VoxelTilingParams::SRV params;
		} &srv;
		struct UAV
		{
			DX12::HLSL::RWTexture3D<float4> output;
		} &uav;
		DX12::HLSL::RWTexture3D<float4>& GetOutput() { return uav.output; }
		DX12::HLSL::Texture3D<float4>& GetAlbedo() { return srv.albedo; }
		DX12::HLSL::Texture3D<float4>& GetNormals() { return srv.normals; }
		DX12::HLSL::Texture3D<float4>& GetLower() { return srv.lower; }
		DX12::HLSL::TextureCube<float4>& GetTex_cube() { return srv.tex_cube; }
		PSSMDataGlobal MapPssmGlobal() { return PSSMDataGlobal(srv.pssmGlobal); }
		VoxelTilingParams MapParams() { return VoxelTilingParams(cb.params,srv.params); }
		VoxelLighting(CB&cb,SRV&srv,UAV&uav) :cb(cb),srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}

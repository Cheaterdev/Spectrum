#pragma once
#include "VoxelTilingParams.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelMipMap
	{
		struct CB
		{
			VoxelTilingParams::CB params;
		} &cb;
		struct SRV
		{
			Render::HLSL::Texture3D<float4> SrcMip;
			VoxelTilingParams::SRV params;
		} &srv;
		struct UAV
		{
			Render::HLSL::RWTexture3D<float4> OutMips[3];
		} &uav;
		Render::HLSL::RWTexture3D<float4>* GetOutMips() { return uav.OutMips; }
		Render::HLSL::Texture3D<float4>& GetSrcMip() { return srv.SrcMip; }
		VoxelTilingParams MapParams() { return VoxelTilingParams(cb.params,srv.params); }
		VoxelMipMap(CB&cb,SRV&srv,UAV&uav) :cb(cb),srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}

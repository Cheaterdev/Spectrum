#pragma once
#include "VoxelTilingParams.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelCopy
	{
		struct CB
		{
			VoxelTilingParams::CB params;
		} &cb;
		struct SRV
		{
			Render::HLSL::Texture3D<float4> Source[2];
			VoxelTilingParams::SRV params;
		} &srv;
		struct UAV
		{
			Render::HLSL::RWTexture3D<float4> Target[2];
		} &uav;
		Render::HLSL::RWTexture3D<float4>* GetTarget() { return uav.Target; }
		Render::HLSL::Texture3D<float4>* GetSource() { return srv.Source; }
		VoxelTilingParams MapParams() { return VoxelTilingParams(cb.params,srv.params); }
		VoxelCopy(CB&cb,SRV&srv,UAV&uav) :cb(cb),srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}

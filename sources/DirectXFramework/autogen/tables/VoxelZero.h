#pragma once
#include "VoxelTilingParams.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelZero
	{
		struct CB
		{
			VoxelTilingParams::CB params;
		} &cb;
		struct SRV
		{
			VoxelTilingParams::SRV params;
		} &srv;
		struct UAV
		{
			Render::HLSL::RWTexture3D<float4> Target;
		} &uav;
		Render::HLSL::RWTexture3D<float4>& GetTarget() { return uav.Target; }
		VoxelTilingParams MapParams() { return VoxelTilingParams(cb.params,srv.params); }
		VoxelZero(CB&cb,SRV&srv,UAV&uav) :cb(cb),srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}

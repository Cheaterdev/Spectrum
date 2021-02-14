#pragma once
#include "TilingParams.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelBlur
	{
		struct SRV
		{
			Render::HLSL::Texture2D<float4> noisy_output;
			Render::HLSL::Texture2D<float4> prev_result;
			TilingParams::SRV tiling;
		} &srv;
		struct UAV
		{
			Render::HLSL::RWTexture2D<float4> screen_result;
			Render::HLSL::RWTexture2D<float4> gi_result;
		} &uav;
		Render::HLSL::Texture2D<float4>& GetNoisy_output() { return srv.noisy_output; }
		Render::HLSL::Texture2D<float4>& GetPrev_result() { return srv.prev_result; }
		Render::HLSL::RWTexture2D<float4>& GetScreen_result() { return uav.screen_result; }
		Render::HLSL::RWTexture2D<float4>& GetGi_result() { return uav.gi_result; }
		TilingParams MapTiling() { return TilingParams(srv.tiling); }
		VoxelBlur(SRV&srv,UAV&uav) :srv(srv),uav(uav){}
	};
	#pragma pack(pop)
}

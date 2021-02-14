#pragma once
#include "TilingParams.h"
struct DenoiserHistoryFix_srv
{
	Texture2D<float4> color;
	Texture2D<float> frames;
	TilingParams_srv tiling;
};
struct DenoiserHistoryFix_uav
{
	RWTexture2D<float4> target;
};
struct DenoiserHistoryFix
{
	DenoiserHistoryFix_srv srv;
	DenoiserHistoryFix_uav uav;
	Texture2D<float4> GetColor() { return srv.color; }
	Texture2D<float> GetFrames() { return srv.frames; }
	RWTexture2D<float4> GetTarget() { return uav.target; }
	TilingParams GetTiling() { return CreateTilingParams(srv.tiling); }

};
 const DenoiserHistoryFix CreateDenoiserHistoryFix(DenoiserHistoryFix_srv srv,DenoiserHistoryFix_uav uav)
{
	const DenoiserHistoryFix result = {srv,uav
	};
	return result;
}

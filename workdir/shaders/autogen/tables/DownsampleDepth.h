#pragma once
struct DownsampleDepth_srv
{
	Texture2D<float> srcTex;
};
struct DownsampleDepth_uav
{
	RWTexture2D<float> targetTex;
};
struct DownsampleDepth
{
	DownsampleDepth_srv srv;
	DownsampleDepth_uav uav;
	Texture2D<float> GetSrcTex() { return srv.srcTex; }
	RWTexture2D<float> GetTargetTex() { return uav.targetTex; }

};
 const DownsampleDepth CreateDownsampleDepth(DownsampleDepth_srv srv,DownsampleDepth_uav uav)
{
	const DownsampleDepth result = {srv,uav
	};
	return result;
}

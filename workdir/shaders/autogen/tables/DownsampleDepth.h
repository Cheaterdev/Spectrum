#pragma once
struct DownsampleDepth_srv
{
	uint srcTex; // Texture2D<float>
};
struct DownsampleDepth_uav
{
	uint targetTex; // RWTexture2D<float>
};
struct DownsampleDepth
{
	DownsampleDepth_srv srv;
	DownsampleDepth_uav uav;
	Texture2D<float> GetSrcTex() { return ResourceDescriptorHeap[srv.srcTex]; }
	RWTexture2D<float> GetTargetTex() { return ResourceDescriptorHeap[uav.targetTex]; }

};
 const DownsampleDepth CreateDownsampleDepth(DownsampleDepth_srv srv,DownsampleDepth_uav uav)
{
	const DownsampleDepth result = {srv,uav
	};
	return result;
}

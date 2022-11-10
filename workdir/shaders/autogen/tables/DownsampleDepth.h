#pragma once
struct DownsampleDepth
{
	uint srcTex; // Texture2D<float>
	uint targetTex; // RWTexture2D<float>
	Texture2D<float> GetSrcTex() { return ResourceDescriptorHeap[srcTex]; }
	RWTexture2D<float> GetTargetTex() { return ResourceDescriptorHeap[targetTex]; }
};

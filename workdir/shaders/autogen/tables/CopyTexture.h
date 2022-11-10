#pragma once
struct CopyTexture
{
	uint srcTex; // Texture2D<float4>
	Texture2D<float4> GetSrcTex() { return ResourceDescriptorHeap[srcTex]; }
};

#pragma once
#include "sig_hlsl.hlsl"
struct CopyTexture
{
	uint srcTex; // Texture2D<float4>
	Texture2D<float4> GetSrcTex() { return ResourceDescriptorHeap[srcTex]; }
};
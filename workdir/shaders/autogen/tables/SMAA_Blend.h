#pragma once
#include "sig_hlsl.hlsl"
struct SMAA_Blend
{
	uint blendTex; // Texture2D<float4>
	Texture2D<float4> GetBlendTex() { return ResourceDescriptorHeap[blendTex]; }
};
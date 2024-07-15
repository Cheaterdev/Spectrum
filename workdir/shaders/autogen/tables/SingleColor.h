#pragma once
#include "sig_hlsl.hlsl"
struct SingleColor
{
	uint color; // RenderTarget<float4>
	Texture2D<float4> GetColor() { return ResourceDescriptorHeap[color]; }
};
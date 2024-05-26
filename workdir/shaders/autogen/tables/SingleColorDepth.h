#pragma once
#include "sig_hlsl.hlsl"
struct SingleColorDepth
{
	uint color; // RenderTarget<float4>
	uint depth; // DepthStencil<float>
	Texture2D<float4> GetColor() { return ResourceDescriptorHeap[color]; }
	Texture2D<float> GetDepth() { return ResourceDescriptorHeap[depth]; }
};

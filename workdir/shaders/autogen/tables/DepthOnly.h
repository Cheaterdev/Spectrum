#pragma once
#include "sig_hlsl.hlsl"
struct DepthOnly
{
	uint depth; // DepthStencil<float>
	Texture2D<float> GetDepth() { return ResourceDescriptorHeap[depth]; }
};
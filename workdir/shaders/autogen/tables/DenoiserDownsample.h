#pragma once
struct DenoiserDownsample
{
	uint color; // Texture2D<float4>
	uint depth; // Texture2D<float>
	Texture2D<float4> GetColor() { return ResourceDescriptorHeap[color]; }
	Texture2D<float> GetDepth() { return ResourceDescriptorHeap[depth]; }
};

#pragma once
struct GBufferDownsampleRT
{
	uint depth; // RenderTarget<float>
	uint color; // RenderTarget<float4>
	Texture2D<float> GetDepth() { return ResourceDescriptorHeap[depth]; }
	Texture2D<float4> GetColor() { return ResourceDescriptorHeap[color]; }
};

#pragma once
struct FontRendering
{
	uint tex0; // Texture2D<float>
	uint positions; // Buffer<float4>
	Texture2D<float> GetTex0() { return ResourceDescriptorHeap[tex0]; }
	Buffer<float4> GetPositions() { return ResourceDescriptorHeap[positions]; }
};

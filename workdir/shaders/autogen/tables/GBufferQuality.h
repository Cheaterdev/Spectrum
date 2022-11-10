#pragma once
struct GBufferQuality
{
	uint ref; // Texture2D<float4>
	Texture2D<float4> GetRef() { return ResourceDescriptorHeap[ref]; }
};

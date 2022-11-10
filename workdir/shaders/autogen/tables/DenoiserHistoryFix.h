#pragma once
struct DenoiserHistoryFix
{
	uint color; // Texture2D<float4>
	uint frames; // Texture2D<float>
	uint target; // RWTexture2D<float4>
	Texture2D<float4> GetColor() { return ResourceDescriptorHeap[color]; }
	Texture2D<float> GetFrames() { return ResourceDescriptorHeap[frames]; }
	RWTexture2D<float4> GetTarget() { return ResourceDescriptorHeap[target]; }
};

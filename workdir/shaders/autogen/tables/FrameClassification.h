#pragma once
struct FrameClassification
{
	uint frames; // Texture2D<float>
	uint hi; // AppendStructuredBuffer<uint2>
	uint low; // AppendStructuredBuffer<uint2>
	Texture2D<float> GetFrames() { return ResourceDescriptorHeap[frames]; }
	AppendStructuredBuffer<uint2> GetHi() { return ResourceDescriptorHeap[hi]; }
	AppendStructuredBuffer<uint2> GetLow() { return ResourceDescriptorHeap[low]; }
};

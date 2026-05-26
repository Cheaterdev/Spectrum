#pragma once
#include "sig_hlsl.hlsl"
struct FrameGraph_Debug_Texture2D
{
	uint2 sourceSize; // uint2
	float2 scale; // float2
	float2 offset; // float2
	uint source; // Texture2D<float4>
	uint2 GetSourceSize() { return sourceSize; }
	float2 GetScale() { return scale; }
	float2 GetOffset() { return offset; }
	Texture2D<float4> GetSource() { return ResourceDescriptorHeap[source]; }
};
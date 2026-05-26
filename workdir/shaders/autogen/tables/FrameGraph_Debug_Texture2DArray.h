#pragma once
#include "sig_hlsl.hlsl"
struct FrameGraph_Debug_Texture2DArray
{
	uint2 sourceSize; // uint2
	float2 scale; // float2
	float2 offset; // float2
	uint source; // Texture2DArray<float4>
	uint2 GetSourceSize() { return sourceSize; }
	float2 GetScale() { return scale; }
	float2 GetOffset() { return offset; }
	Texture2DArray<float4> GetSource() { return ResourceDescriptorHeap[source]; }
};
#pragma once
#include "sig_hlsl.hlsl"
struct FrameGraph_Debug_Texture2D
{
	float2 scale; // float2
	float2 offset; // float2
	uint source; // Texture2D<float4>
	float2 GetScale() { return scale; }
	float2 GetOffset() { return offset; }
	Texture2D<float4> GetSource() { return ResourceDescriptorHeap[source]; }
};
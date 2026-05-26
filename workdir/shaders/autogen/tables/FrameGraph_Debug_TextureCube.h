#pragma once
#include "sig_hlsl.hlsl"
struct FrameGraph_Debug_TextureCube
{
	uint2 sourceSize; // uint2
	uint source; // TextureCube<float4>
	uint2 GetSourceSize() { return sourceSize; }
	TextureCube<float4> GetSource() { return ResourceDescriptorHeap[source]; }
};
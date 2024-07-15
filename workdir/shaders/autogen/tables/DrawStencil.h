#pragma once
#include "sig_hlsl.hlsl"
struct DrawStencil
{
	uint vertices; // StructuredBuffer<float4>
	StructuredBuffer<float4> GetVertices() { return ResourceDescriptorHeap[vertices]; }
};
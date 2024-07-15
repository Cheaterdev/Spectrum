#pragma once
#include "sig_hlsl.hlsl"
#include "VSLine.h"
struct LineRender
{
	uint vb; // StructuredBuffer<VSLine>
	StructuredBuffer<VSLine> GetVb() { return ResourceDescriptorHeap[vb]; }
};
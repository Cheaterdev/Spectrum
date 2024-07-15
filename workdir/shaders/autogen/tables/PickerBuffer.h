#pragma once
#include "sig_hlsl.hlsl"
struct PickerBuffer
{
	uint viewBuffer; // RWStructuredBuffer<uint>
	RWStructuredBuffer<uint> GetViewBuffer() { return ResourceDescriptorHeap[viewBuffer]; }
};
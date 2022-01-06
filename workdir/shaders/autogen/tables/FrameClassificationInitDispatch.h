#pragma once
#include "DispatchArguments.h"
struct FrameClassificationInitDispatch
{
	uint hi_counter; // StructuredBuffer<uint>
	uint low_counter; // StructuredBuffer<uint>
	uint hi_dispatch_data; // RWStructuredBuffer<DispatchArguments>
	uint low_dispatch_data; // RWStructuredBuffer<DispatchArguments>
	StructuredBuffer<uint> GetHi_counter() { return ResourceDescriptorHeap[hi_counter]; }
	StructuredBuffer<uint> GetLow_counter() { return ResourceDescriptorHeap[low_counter]; }
	RWStructuredBuffer<DispatchArguments> GetHi_dispatch_data() { return ResourceDescriptorHeap[hi_dispatch_data]; }
	RWStructuredBuffer<DispatchArguments> GetLow_dispatch_data() { return ResourceDescriptorHeap[low_dispatch_data]; }
};

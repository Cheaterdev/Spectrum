#pragma once
#include "DispatchArguments.h"
struct InitDispatch
{
	uint counter; // RWStructuredBuffer<uint>
	uint dispatch_data; // RWStructuredBuffer<DispatchArguments>
	RWStructuredBuffer<uint> GetCounter() { return ResourceDescriptorHeap[counter]; }
	RWStructuredBuffer<DispatchArguments> GetDispatch_data() { return ResourceDescriptorHeap[dispatch_data]; }
};

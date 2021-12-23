#pragma once
#include "DispatchArguments.h"
struct InitDispatch_uav
{
	uint counter; // RWStructuredBuffer<uint>
	uint dispatch_data; // RWStructuredBuffer<DispatchArguments>
};
struct InitDispatch
{
	InitDispatch_uav uav;
	RWStructuredBuffer<uint> GetCounter() { return ResourceDescriptorHeap[uav.counter]; }
	RWStructuredBuffer<DispatchArguments> GetDispatch_data() { return ResourceDescriptorHeap[uav.dispatch_data]; }

};
 const InitDispatch CreateInitDispatch(InitDispatch_uav uav)
{
	const InitDispatch result = {uav
	};
	return result;
}

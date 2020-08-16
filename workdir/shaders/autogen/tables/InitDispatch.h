#pragma once
#include "DispatchArguments.h"
struct InitDispatch_uav
{
	RWStructuredBuffer<uint> counter;
	RWStructuredBuffer<DispatchArguments> dispatch_data;
};
struct InitDispatch
{
	InitDispatch_uav uav;
	RWStructuredBuffer<uint> GetCounter() { return uav.counter; }
	RWStructuredBuffer<DispatchArguments> GetDispatch_data() { return uav.dispatch_data; }
};
 const InitDispatch CreateInitDispatch(InitDispatch_uav uav)
{
	const InitDispatch result = {uav
	};
	return result;
}

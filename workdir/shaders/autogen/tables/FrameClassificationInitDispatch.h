#pragma once
#include "DispatchArguments.h"
struct FrameClassificationInitDispatch_srv
{
	StructuredBuffer<uint> hi_counter;
	StructuredBuffer<uint> low_counter;
};
struct FrameClassificationInitDispatch_uav
{
	RWStructuredBuffer<DispatchArguments> hi_dispatch_data;
	RWStructuredBuffer<DispatchArguments> low_dispatch_data;
};
struct FrameClassificationInitDispatch
{
	FrameClassificationInitDispatch_srv srv;
	FrameClassificationInitDispatch_uav uav;
	StructuredBuffer<uint> GetHi_counter() { return srv.hi_counter; }
	StructuredBuffer<uint> GetLow_counter() { return srv.low_counter; }
	RWStructuredBuffer<DispatchArguments> GetHi_dispatch_data() { return uav.hi_dispatch_data; }
	RWStructuredBuffer<DispatchArguments> GetLow_dispatch_data() { return uav.low_dispatch_data; }

};
 const FrameClassificationInitDispatch CreateFrameClassificationInitDispatch(FrameClassificationInitDispatch_srv srv,FrameClassificationInitDispatch_uav uav)
{
	const FrameClassificationInitDispatch result = {srv,uav
	};
	return result;
}

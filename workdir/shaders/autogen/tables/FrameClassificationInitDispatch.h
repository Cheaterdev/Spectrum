#pragma once
#include "DispatchArguments.h"
struct FrameClassificationInitDispatch_srv
{
	uint hi_counter; // StructuredBuffer<uint>
	uint low_counter; // StructuredBuffer<uint>
};
struct FrameClassificationInitDispatch_uav
{
	uint hi_dispatch_data; // RWStructuredBuffer<DispatchArguments>
	uint low_dispatch_data; // RWStructuredBuffer<DispatchArguments>
};
struct FrameClassificationInitDispatch
{
	FrameClassificationInitDispatch_srv srv;
	FrameClassificationInitDispatch_uav uav;
	StructuredBuffer<uint> GetHi_counter() { return ResourceDescriptorHeap[srv.hi_counter]; }
	StructuredBuffer<uint> GetLow_counter() { return ResourceDescriptorHeap[srv.low_counter]; }
	RWStructuredBuffer<DispatchArguments> GetHi_dispatch_data() { return ResourceDescriptorHeap[uav.hi_dispatch_data]; }
	RWStructuredBuffer<DispatchArguments> GetLow_dispatch_data() { return ResourceDescriptorHeap[uav.low_dispatch_data]; }

};
 const FrameClassificationInitDispatch CreateFrameClassificationInitDispatch(FrameClassificationInitDispatch_srv srv,FrameClassificationInitDispatch_uav uav)
{
	const FrameClassificationInitDispatch result = {srv,uav
	};
	return result;
}

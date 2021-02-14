#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FrameClassificationInitDispatch.h"
StructuredBuffer<uint> srv_5_0: register(t0, space5);
StructuredBuffer<uint> srv_5_1: register(t1, space5);
RWStructuredBuffer<DispatchArguments> uav_5_0: register(u0, space5);
RWStructuredBuffer<DispatchArguments> uav_5_1: register(u1, space5);
FrameClassificationInitDispatch CreateFrameClassificationInitDispatch()
{
	FrameClassificationInitDispatch result;
	result.srv.hi_counter = srv_5_0;
	result.srv.low_counter = srv_5_1;
	result.uav.hi_dispatch_data = uav_5_0;
	result.uav.low_dispatch_data = uav_5_1;
	return result;
}
#ifndef NO_GLOBAL
static const FrameClassificationInitDispatch frameClassificationInitDispatch_global = CreateFrameClassificationInitDispatch();
const FrameClassificationInitDispatch GetFrameClassificationInitDispatch(){ return frameClassificationInitDispatch_global; }
#endif
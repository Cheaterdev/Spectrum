#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FrameClassificationInitDispatch.h"
StructuredBuffer<uint> srv_6_0: register(t0, space6);
StructuredBuffer<uint> srv_6_1: register(t1, space6);
RWStructuredBuffer<DispatchArguments> uav_6_0: register(u0, space6);
RWStructuredBuffer<DispatchArguments> uav_6_1: register(u1, space6);
struct Pass_FrameClassificationInitDispatch
{
uint srv_0;
uint srv_1;
uint uav_0;
uint uav_1;
};
ConstantBuffer<Pass_FrameClassificationInitDispatch> pass_FrameClassificationInitDispatch: register( b2, space6);
const FrameClassificationInitDispatch CreateFrameClassificationInitDispatch()
{
	FrameClassificationInitDispatch result;
	result.srv.hi_counter = (pass_FrameClassificationInitDispatch.srv_0 );
	result.srv.low_counter = (pass_FrameClassificationInitDispatch.srv_1 );
	result.uav.hi_dispatch_data = (pass_FrameClassificationInitDispatch.uav_0 );
	result.uav.low_dispatch_data = (pass_FrameClassificationInitDispatch.uav_1 );
	return result;
}
#ifndef NO_GLOBAL
static const FrameClassificationInitDispatch frameClassificationInitDispatch_global = CreateFrameClassificationInitDispatch();
const FrameClassificationInitDispatch GetFrameClassificationInitDispatch(){ return frameClassificationInitDispatch_global; }
#endif

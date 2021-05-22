#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/InitDispatch.h"
RWStructuredBuffer<uint> uav_5_0: register(u0, space5);
RWStructuredBuffer<DispatchArguments> uav_5_1: register(u1, space5);
struct Pass_InitDispatch
{
uint uav_0;
uint uav_1;
};
ConstantBuffer<Pass_InitDispatch> pass_InitDispatch: register( b2, space5);
const InitDispatch CreateInitDispatch()
{
	InitDispatch result;
	Pass_InitDispatch pass;
	result.uav.counter = uav_5_0;
	result.uav.dispatch_data = uav_5_1;
	return result;
}
#ifndef NO_GLOBAL
static const InitDispatch initDispatch_global = CreateInitDispatch();
const InitDispatch GetInitDispatch(){ return initDispatch_global; }
#endif

#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/InitDispatch.h"
RWStructuredBuffer<uint> uav_4_0: register(u0, space4);
RWStructuredBuffer<DispatchArguments> uav_4_1: register(u1, space4);
InitDispatch CreateInitDispatch()
{
	InitDispatch result;
	result.uav.counter = uav_4_0;
	result.uav.dispatch_data = uav_4_1;
	return result;
}
#ifndef NO_GLOBAL
static const InitDispatch initDispatch_global = CreateInitDispatch();
const InitDispatch GetInitDispatch(){ return initDispatch_global; }
#endif

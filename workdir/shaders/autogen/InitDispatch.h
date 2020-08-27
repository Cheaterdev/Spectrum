#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/InitDispatch.h"
RWStructuredBuffer<uint> uav_3_0: register(u0, space3);
RWStructuredBuffer<DispatchArguments> uav_3_1: register(u1, space3);
InitDispatch CreateInitDispatch()
{
	InitDispatch result;
	result.uav.counter = uav_3_0;
	result.uav.dispatch_data = uav_3_1;
	return result;
}
#ifndef NO_GLOBAL
static const InitDispatch initDispatch_global = CreateInitDispatch();
const InitDispatch GetInitDispatch(){ return initDispatch_global; }
#endif

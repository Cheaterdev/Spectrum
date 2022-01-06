#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/InitDispatch.h"
ConstantBuffer<InitDispatch> pass_InitDispatch: register( b2, space5);
const InitDispatch CreateInitDispatch()
{
	return pass_InitDispatch;
}
#ifndef NO_GLOBAL
static const InitDispatch initDispatch_global = CreateInitDispatch();
const InitDispatch GetInitDispatch(){ return initDispatch_global; }
#endif

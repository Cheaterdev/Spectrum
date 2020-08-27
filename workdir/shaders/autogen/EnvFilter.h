#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/EnvFilter.h"
ConstantBuffer<EnvFilter_cb> cb_3_0:register(b0,space3);
EnvFilter CreateEnvFilter()
{
	EnvFilter result;
	result.cb = cb_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const EnvFilter envFilter_global = CreateEnvFilter();
const EnvFilter GetEnvFilter(){ return envFilter_global; }
#endif

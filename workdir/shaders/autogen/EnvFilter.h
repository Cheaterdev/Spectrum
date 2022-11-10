#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/EnvFilter.h"
ConstantBuffer<EnvFilter> pass_EnvFilter: register( b2, space5);
const EnvFilter CreateEnvFilter()
{
	return pass_EnvFilter;
}
#ifndef NO_GLOBAL
static const EnvFilter envFilter_global = CreateEnvFilter();
const EnvFilter GetEnvFilter(){ return envFilter_global; }
#endif

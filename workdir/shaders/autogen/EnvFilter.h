#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/EnvFilter.h"
ConstantBuffer<EnvFilter_cb> cb_5_0:register(b0,space5);
struct Pass_EnvFilter
{
};
ConstantBuffer<Pass_EnvFilter> pass_EnvFilter: register( b2, space5);
const EnvFilter CreateEnvFilter()
{
	EnvFilter result;
	result.cb = cb_5_0;
	return result;
}
#ifndef NO_GLOBAL
static const EnvFilter envFilter_global = CreateEnvFilter();
const EnvFilter GetEnvFilter(){ return envFilter_global; }
#endif

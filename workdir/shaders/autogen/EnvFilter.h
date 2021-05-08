#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/EnvFilter.h"
ConstantBuffer<EnvFilter_cb> cb_4_0:register(b0,space4);
struct Pass_EnvFilter
{
};
ConstantBuffer<Pass_EnvFilter> pass_EnvFilter: register( b2, space4);
const EnvFilter CreateEnvFilter()
{
	EnvFilter result;
	Pass_EnvFilter pass;
	result.cb = cb_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const EnvFilter envFilter_global = CreateEnvFilter();
const EnvFilter GetEnvFilter(){ return envFilter_global; }
#endif

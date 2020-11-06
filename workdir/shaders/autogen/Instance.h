#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Instance.h"
ConstantBuffer<Instance_cb> cb_5_0:register(b0,space5);
Instance CreateInstance()
{
	Instance result;
	result.cb = cb_5_0;
	return result;
}
#ifndef NO_GLOBAL
static const Instance instance_global = CreateInstance();
const Instance GetInstance(){ return instance_global; }
#endif

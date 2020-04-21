#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Instance.h"
ConstantBuffer<Instance_cb> cb_4_0:register(b0,space4);
Instance CreateInstance()
{
	Instance result;
	result.cb = cb_4_0;
	return result;
}
static const Instance instance_global = CreateInstance();
const Instance GetInstance(){ return instance_global; }

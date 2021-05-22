#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Instance.h"
ConstantBuffer<Instance_cb> cb_6_0:register(b0,space6);
struct Pass_Instance
{
};
ConstantBuffer<Pass_Instance> pass_Instance: register( b2, space6);
const Instance CreateInstance()
{
	Instance result;
	Pass_Instance pass;
	result.cb = cb_6_0;
	return result;
}
#ifndef NO_GLOBAL
static const Instance instance_global = CreateInstance();
const Instance GetInstance(){ return instance_global; }
#endif

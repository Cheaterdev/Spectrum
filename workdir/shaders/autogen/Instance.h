#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Instance.h"
ConstantBuffer<Instance> pass_Instance: register( b2, space6);
const Instance CreateInstance()
{
	return pass_Instance;
}
#ifndef NO_GLOBAL
static const Instance instance_global = CreateInstance();
const Instance GetInstance(){ return instance_global; }
#endif

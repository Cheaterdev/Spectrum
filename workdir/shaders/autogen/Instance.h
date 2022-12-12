#ifndef SLOT_7
	#define SLOT_7
#else
	#error Slot 7 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Instance.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_Instance: register( b2, space7);
 ConstantBuffer<Instance> CreateInstance()
{
	return ResourceDescriptorHeap[pass_Instance.offset];
}
#ifndef NO_GLOBAL
static const Instance instance_global = CreateInstance();
const Instance GetInstance(){ return instance_global; }
#endif

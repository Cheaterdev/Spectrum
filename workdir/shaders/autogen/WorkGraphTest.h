#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif

#include "layout/DefaultLayout.h"
#include "tables/WorkGraphTest.h"

#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif

ConstantBuffer< CB > pass_WorkGraphTest: register( b2, space4);

ConstantBuffer<WorkGraphTest> CreateWorkGraphTest()
{
	return ResourceDescriptorHeap[pass_WorkGraphTest.offset];
}
			
#ifndef NO_GLOBAL
static const WorkGraphTest workGraphTest_global = CreateWorkGraphTest();
const WorkGraphTest GetWorkGraphTest(){ return workGraphTest_global; }
#endif
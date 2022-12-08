#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Test.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_Test: register( b2, space4);
 ConstantBuffer<Test> CreateTest()
{
	return ResourceDescriptorHeap[pass_Test.offset];
}
#ifndef NO_GLOBAL
static const Test test_global = CreateTest();
const Test GetTest(){ return test_global; }
#endif

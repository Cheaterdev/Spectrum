#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/Test.h"
ConstantBuffer<Test> pass_Test: register( b2, space4);
const Test CreateTest()
{
	return pass_Test;
}
#ifndef NO_GLOBAL
static const Test test_global = CreateTest();
const Test GetTest(){ return test_global; }
#endif

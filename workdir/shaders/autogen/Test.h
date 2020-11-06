#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
Texture2D bindless[]: register(t0, space3);
#include "tables/Test.h"
ConstantBuffer<Test_cb> cb_3_0:register(b0,space3);
Test CreateTest()
{
	Test result;
	result.cb = cb_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const Test test_global = CreateTest();
const Test GetTest(){ return test_global; }
#endif

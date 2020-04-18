#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
Texture2D bindless[]: register(t0, space2);
#include "tables/Test.h"
ConstantBuffer<Test_cb> cb_2_0:register(b0,space2);
Test CreateTest()
{
	Test result;
	result.cb = cb_2_0;
	return result;
}
static const Test test_global = CreateTest();
const Test GetTest(){ return test_global; }

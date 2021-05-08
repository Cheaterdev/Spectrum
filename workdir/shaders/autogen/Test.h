#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
Texture2D<float4> bindless[]: register(t0, space3);
#include "tables/Test.h"
ConstantBuffer<Test_cb> cb_3_0:register(b0,space3);
struct Pass_Test
{
};
ConstantBuffer<Pass_Test> pass_Test: register( b2, space3);
const Test CreateTest()
{
	Test result;
	Pass_Test pass;
	result.cb = cb_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const Test test_global = CreateTest();
const Test GetTest(){ return test_global; }
#endif

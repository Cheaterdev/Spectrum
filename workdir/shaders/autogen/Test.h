#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
Texture2D<float4> bindless[]: register(t0, space4);
#include "tables/Test.h"
ConstantBuffer<Test_cb> cb_4_0:register(b0,space4);
struct Pass_Test
{
};
ConstantBuffer<Pass_Test> pass_Test: register( b2, space4);
const Test CreateTest()
{
	Test result;
	Pass_Test pass;
	result.cb = cb_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const Test test_global = CreateTest();
const Test GetTest(){ return test_global; }
#endif

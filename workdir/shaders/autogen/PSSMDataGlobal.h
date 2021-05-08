#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMDataGlobal.h"
Texture2D<float> srv_4_0: register(t0, space4);
StructuredBuffer<Camera> srv_4_1: register(t1, space4);
struct Pass_PSSMDataGlobal
{
uint srv_0;
uint srv_1;
};
ConstantBuffer<Pass_PSSMDataGlobal> pass_PSSMDataGlobal: register( b2, space4);
const PSSMDataGlobal CreatePSSMDataGlobal()
{
	PSSMDataGlobal result;
	Pass_PSSMDataGlobal pass;
	result.srv.light_buffer = srv_4_0;
	result.srv.light_camera = srv_4_1;
	return result;
}
#ifndef NO_GLOBAL
static const PSSMDataGlobal pSSMDataGlobal_global = CreatePSSMDataGlobal();
const PSSMDataGlobal GetPSSMDataGlobal(){ return pSSMDataGlobal_global; }
#endif

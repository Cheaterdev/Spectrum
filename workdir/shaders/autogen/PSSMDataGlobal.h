#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/PSSMDataGlobal.h"
Texture2D<float> srv_5_0: register(t0, space5);
StructuredBuffer<Camera> srv_5_1: register(t1, space5);
struct Pass_PSSMDataGlobal
{
uint srv_0;
uint srv_1;
};
ConstantBuffer<Pass_PSSMDataGlobal> pass_PSSMDataGlobal: register( b2, space5);
const PSSMDataGlobal CreatePSSMDataGlobal()
{
	PSSMDataGlobal result;
	Pass_PSSMDataGlobal pass;
	result.srv.light_buffer = srv_5_0;
	result.srv.light_camera = srv_5_1;
	return result;
}
#ifndef NO_GLOBAL
static const PSSMDataGlobal pSSMDataGlobal_global = CreatePSSMDataGlobal();
const PSSMDataGlobal GetPSSMDataGlobal(){ return pSSMDataGlobal_global; }
#endif

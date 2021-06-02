#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherPipelineGlobal.h"
StructuredBuffer<uint> srv_4_0: register(t0, space4);
Buffer<uint> srv_4_1: register(t1, space4);
struct Pass_GatherPipelineGlobal
{
uint srv_0;
uint srv_1;
};
ConstantBuffer<Pass_GatherPipelineGlobal> pass_GatherPipelineGlobal: register( b2, space4);
const GatherPipelineGlobal CreateGatherPipelineGlobal()
{
	GatherPipelineGlobal result;
	result.srv.meshes_count = srv_4_0;
	result.srv.commands = srv_4_1;
	return result;
}
#ifndef NO_GLOBAL
static const GatherPipelineGlobal gatherPipelineGlobal_global = CreateGatherPipelineGlobal();
const GatherPipelineGlobal GetGatherPipelineGlobal(){ return gatherPipelineGlobal_global; }
#endif

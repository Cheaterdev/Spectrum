#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherPipelineGlobal.h"
StructuredBuffer<uint> srv_3_0: register(t0, space3);
Buffer<uint> srv_3_1: register(t1, space3);
struct Pass_GatherPipelineGlobal
{
uint srv_0;
uint srv_1;
};
ConstantBuffer<Pass_GatherPipelineGlobal> pass_GatherPipelineGlobal: register( b2, space3);
const GatherPipelineGlobal CreateGatherPipelineGlobal()
{
	GatherPipelineGlobal result;
	Pass_GatherPipelineGlobal pass;
	result.srv.meshes_count = srv_3_0;
	result.srv.commands = srv_3_1;
	return result;
}
#ifndef NO_GLOBAL
static const GatherPipelineGlobal gatherPipelineGlobal_global = CreateGatherPipelineGlobal();
const GatherPipelineGlobal GetGatherPipelineGlobal(){ return gatherPipelineGlobal_global; }
#endif

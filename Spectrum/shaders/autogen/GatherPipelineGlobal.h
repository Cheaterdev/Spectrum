#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/GatherPipelineGlobal.h"
ConstantBuffer<GatherPipelineGlobal_cb> cb_3_0:register(b0,space3);
StructuredBuffer<MeshCommandData> srv_3_0: register(t0, space3);
StructuredBuffer<MaterialCommandData> srv_3_1: register(t1, space3);
GatherPipelineGlobal CreateGatherPipelineGlobal()
{
	GatherPipelineGlobal result;
	result.cb = cb_3_0;
	result.srv.meshes = srv_3_0;
	result.srv.materials = srv_3_1;
	return result;
}
static const GatherPipelineGlobal gatherPipelineGlobal_global = CreateGatherPipelineGlobal();
const GatherPipelineGlobal GetGatherPipelineGlobal(){ return gatherPipelineGlobal_global; }

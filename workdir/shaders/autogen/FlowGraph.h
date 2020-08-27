#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FlowGraph.h"
ConstantBuffer<FlowGraph_cb> cb_3_0:register(b0,space3);
FlowGraph CreateFlowGraph()
{
	FlowGraph result;
	result.cb = cb_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const FlowGraph flowGraph_global = CreateFlowGraph();
const FlowGraph GetFlowGraph(){ return flowGraph_global; }
#endif

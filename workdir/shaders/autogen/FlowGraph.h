#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FlowGraph.h"
ConstantBuffer<FlowGraph_cb> cb_4_0:register(b0,space4);
FlowGraph CreateFlowGraph()
{
	FlowGraph result;
	result.cb = cb_4_0;
	return result;
}
static const FlowGraph flowGraph_global = CreateFlowGraph();
const FlowGraph GetFlowGraph(){ return flowGraph_global; }

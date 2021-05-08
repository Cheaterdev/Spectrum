#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FlowGraph.h"
ConstantBuffer<FlowGraph_cb> cb_4_0:register(b0,space4);
struct Pass_FlowGraph
{
};
ConstantBuffer<Pass_FlowGraph> pass_FlowGraph: register( b2, space4);
const FlowGraph CreateFlowGraph()
{
	FlowGraph result;
	Pass_FlowGraph pass;
	result.cb = cb_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const FlowGraph flowGraph_global = CreateFlowGraph();
const FlowGraph GetFlowGraph(){ return flowGraph_global; }
#endif

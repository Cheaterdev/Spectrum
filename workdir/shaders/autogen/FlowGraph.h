#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FlowGraph.h"
ConstantBuffer<FlowGraph> pass_FlowGraph: register( b2, space5);
const FlowGraph CreateFlowGraph()
{
	return pass_FlowGraph;
}
#ifndef NO_GLOBAL
static const FlowGraph flowGraph_global = CreateFlowGraph();
const FlowGraph GetFlowGraph(){ return flowGraph_global; }
#endif

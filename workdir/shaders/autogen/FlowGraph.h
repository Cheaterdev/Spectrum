#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/FlowGraph.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_FlowGraph: register( b2, space5);
 ConstantBuffer<FlowGraph> CreateFlowGraph()
{
	return ResourceDescriptorHeap[pass_FlowGraph.offset];
}
#ifndef NO_GLOBAL
static const FlowGraph flowGraph_global = CreateFlowGraph();
const FlowGraph GetFlowGraph(){ return flowGraph_global; }
#endif

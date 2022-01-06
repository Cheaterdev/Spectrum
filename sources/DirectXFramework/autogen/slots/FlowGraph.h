#pragma once
#include "..\Tables\FlowGraph.h"
namespace Slots {
	struct FlowGraph:public DataHolder<FlowGraph, SlotID::FlowGraph,Table::FlowGraph,DefaultLayout::Instance1>
	{
		FlowGraph() = default;
	};
}

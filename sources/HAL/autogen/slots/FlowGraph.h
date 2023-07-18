#pragma once
#include "..\Tables\FlowGraph.h"
namespace Slots {
	struct FlowGraph:public DataHolder<FlowGraph, SlotID::FlowGraph,Table::FlowGraph,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FlowGraph() = default;
	};
}

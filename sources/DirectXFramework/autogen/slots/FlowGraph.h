#pragma once
#include "..\Tables\FlowGraph.h"
namespace Slots {
	struct FlowGraph:public DataHolder<FlowGraph, SlotID::FlowGraph,Table::FlowGraph,DefaultLayout::Instance1>
	{
		CB cb;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::CB_ID};
		FlowGraph(): DataHolder(cb){}
		FlowGraph(const FlowGraph&other): DataHolder(cb){cb = other.cb;}
	};
}

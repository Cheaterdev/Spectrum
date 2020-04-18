#pragma once
#include "..\Tables\FlowGraph.h"
namespace Slots {
	struct FlowGraph:public DataHolder<Table::FlowGraph,DefaultLayout::Instance1>
	{
		CB cb;
		FlowGraph(): DataHolder(cb){}
	};
}

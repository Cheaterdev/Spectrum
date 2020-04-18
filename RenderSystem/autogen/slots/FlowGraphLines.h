#pragma once
#include "..\Tables\FlowGraphLines.h"
namespace Slots {
	struct FlowGraphLines:public DataHolder<Table::FlowGraphLines,DefaultLayout::Instance1>
	{
		CB cb;
		FlowGraphLines(): DataHolder(cb){}
	};
}

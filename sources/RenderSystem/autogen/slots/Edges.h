#pragma once
#include "..\Tables\Edges.h"
namespace Slots {
	struct Edges:public DataHolder<Table::Edges,DefaultLayout::Instance0>
	{
		SRV srv;
		Edges(): DataHolder(srv){}
		Edges(const Edges&other): DataHolder(srv){srv = other.srv;}
	};
}

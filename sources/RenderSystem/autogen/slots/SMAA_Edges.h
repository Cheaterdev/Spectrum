#pragma once
#include "..\Tables\SMAA_Edges.h"
namespace Slots {
	struct SMAA_Edges:public DataHolder<Table::SMAA_Edges,DefaultLayout::Instance0>
	{
		CB cb;
		SRV srv;
		SMAA_Edges(): DataHolder(cb,srv){}
		SMAA_Edges(const SMAA_Edges&other): DataHolder(cb,srv){cb = other.cb;srv = other.srv;}
	};
}

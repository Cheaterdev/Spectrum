#pragma once
#include "..\Tables\LineRender.h"
namespace Slots {
	struct LineRender:public DataHolder<Table::LineRender,DefaultLayout::Instance0>
	{
		SRV srv;
		LineRender(): DataHolder(srv){}
		LineRender(const LineRender&other): DataHolder(srv){srv = other.srv;}
	};
}

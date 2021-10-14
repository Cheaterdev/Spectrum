#pragma once
#include "..\Tables\LineRender.h"
namespace Slots {
	struct LineRender:public DataHolder<LineRender, SlotID::LineRender,Table::LineRender,DefaultLayout::Instance0>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::SRV_ID};
		LineRender(): DataHolder(srv){}
		LineRender(const LineRender&other): DataHolder(srv){srv = other.srv;}
	};
}

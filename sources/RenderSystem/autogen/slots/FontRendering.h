#pragma once
#include "..\Tables\FontRendering.h"
namespace Slots {
	struct FontRendering:public DataHolder<FontRendering, SlotID::FontRendering,Table::FontRendering,DefaultLayout::Instance0>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::SRV_ID};
		FontRendering(): DataHolder(srv){}
		FontRendering(const FontRendering&other): DataHolder(srv){srv = other.srv;}
	};
}

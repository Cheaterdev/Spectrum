#pragma once
#include "..\Tables\FontRenderingGlyphs.h"
namespace Slots {
	struct FontRenderingGlyphs:public DataHolder<FontRenderingGlyphs, SlotID::FontRenderingGlyphs,Table::FontRenderingGlyphs,DefaultLayout::Instance2>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance2::SRV_ID};
		FontRenderingGlyphs(): DataHolder(srv){}
		FontRenderingGlyphs(const FontRenderingGlyphs&other): DataHolder(srv){srv = other.srv;}
	};
}

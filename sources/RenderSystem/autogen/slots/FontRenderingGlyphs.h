#pragma once
#include "..\Tables\FontRenderingGlyphs.h"
namespace Slots {
	struct FontRenderingGlyphs:public DataHolder<SlotID::FontRenderingGlyphs,Table::FontRenderingGlyphs,DefaultLayout::Instance2>
	{
		SRV srv;
		FontRenderingGlyphs(): DataHolder(srv){}
		FontRenderingGlyphs(const FontRenderingGlyphs&other): DataHolder(srv){srv = other.srv;}
	};
}

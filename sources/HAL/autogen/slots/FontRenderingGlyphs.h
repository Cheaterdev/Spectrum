#pragma once
#include "..\Tables\FontRenderingGlyphs.h"
namespace Slots {
	struct FontRenderingGlyphs:public DataHolder<FontRenderingGlyphs, SlotID::FontRenderingGlyphs,Table::FontRenderingGlyphs,DefaultLayout::Instance2>
	{
		FontRenderingGlyphs() = default;
	};
}

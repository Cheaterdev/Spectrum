#pragma once
#include "..\Tables\FontRenderingGlyphs.h"
namespace Slots {
	struct FontRenderingGlyphs:public DataHolder<FontRenderingGlyphs, SlotID::FontRenderingGlyphs,Table::FontRenderingGlyphs,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FontRenderingGlyphs() = default;
	};
}

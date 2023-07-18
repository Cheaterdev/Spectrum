#pragma once
#include "..\Tables\FontRendering.h"
namespace Slots {
	struct FontRendering:public DataHolder<FontRendering, SlotID::FontRendering,Table::FontRendering,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FontRendering() = default;
	};
}

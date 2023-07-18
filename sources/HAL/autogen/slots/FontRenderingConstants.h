#pragma once
#include "..\Tables\FontRenderingConstants.h"
namespace Slots {
	struct FontRenderingConstants:public DataHolder<FontRenderingConstants, SlotID::FontRenderingConstants,Table::FontRenderingConstants,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FontRenderingConstants() = default;
	};
}

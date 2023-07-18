#pragma once
#include "..\Tables\ColorRect.h"
namespace Slots {
	struct ColorRect:public DataHolder<ColorRect, SlotID::ColorRect,Table::ColorRect,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		ColorRect() = default;
	};
}

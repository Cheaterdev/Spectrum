#pragma once
#include "..\Tables\ColorRect.h"
namespace Slots {
	struct ColorRect:public DataHolder<ColorRect, SlotID::ColorRect,Table::ColorRect,DefaultLayout::Instance0>
	{
		ColorRect() = default;
	};
}

#pragma once
#include "..\Tables\Color.h"
namespace Slots {
	struct Color:public DataHolder<Color, SlotID::Color,Table::Color,DefaultLayout::Instance0>
	{
		Color() = default;
	};
}

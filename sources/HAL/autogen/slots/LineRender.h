#pragma once
#include "..\Tables\LineRender.h"
namespace Slots {
	struct LineRender:public DataHolder<LineRender, SlotID::LineRender,Table::LineRender,DefaultLayout::Instance0>
	{
		LineRender() = default;
	};
}

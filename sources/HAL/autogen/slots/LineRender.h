#pragma once
#include "..\Tables\LineRender.h"
namespace Slots {
	struct LineRender:public DataHolder<LineRender, SlotID::LineRender,Table::LineRender,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		LineRender() = default;
	};
}

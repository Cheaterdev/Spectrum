#pragma once
#include "..\Tables\DrawBoxes.h"
namespace Slots {
	struct DrawBoxes:public DataHolder<DrawBoxes, SlotID::DrawBoxes,Table::DrawBoxes,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DrawBoxes() = default;
	};
}

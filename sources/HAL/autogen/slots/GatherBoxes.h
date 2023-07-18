#pragma once
#include "..\Tables\GatherBoxes.h"
namespace Slots {
	struct GatherBoxes:public DataHolder<GatherBoxes, SlotID::GatherBoxes,Table::GatherBoxes,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		GatherBoxes() = default;
	};
}

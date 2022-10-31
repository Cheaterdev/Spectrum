#pragma once
#include "..\Tables\GatherBoxes.h"
namespace Slots {
	struct GatherBoxes:public DataHolder<GatherBoxes, SlotID::GatherBoxes,Table::GatherBoxes,DefaultLayout::Instance1>
	{
		GatherBoxes() = default;
	};
}

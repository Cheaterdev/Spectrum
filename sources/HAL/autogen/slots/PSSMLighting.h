#pragma once
#include "..\Tables\PSSMLighting.h"
namespace Slots {
	struct PSSMLighting:public DataHolder<PSSMLighting, SlotID::PSSMLighting,Table::PSSMLighting,DefaultLayout::Instance2>
	{
		PSSMLighting() = default;
	};
}

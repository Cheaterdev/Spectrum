#pragma once
#include "..\Tables\PSSMLighting.h"
namespace Slots {
	struct PSSMLighting:public DataHolder<PSSMLighting, SlotID::PSSMLighting,Table::PSSMLighting,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		PSSMLighting() = default;
	};
}

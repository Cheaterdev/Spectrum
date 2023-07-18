#pragma once
#include "..\Tables\NinePatch.h"
namespace Slots {
	struct NinePatch:public DataHolder<NinePatch, SlotID::NinePatch,Table::NinePatch,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		NinePatch() = default;
	};
}

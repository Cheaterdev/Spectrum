#pragma once
#include "..\Tables\FSR.h"
namespace Slots {
	struct FSR:public DataHolder<FSR, SlotID::FSR,Table::FSR,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FSR() = default;
	};
}

#pragma once
#include "..\Tables\Countour.h"
namespace Slots {
	struct Countour:public DataHolder<Countour, SlotID::Countour,Table::Countour,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		Countour() = default;
	};
}

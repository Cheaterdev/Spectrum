#pragma once
#include "..\Tables\DenoiserHistoryFix.h"
namespace Slots {
	struct DenoiserHistoryFix:public DataHolder<DenoiserHistoryFix, SlotID::DenoiserHistoryFix,Table::DenoiserHistoryFix,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserHistoryFix() = default;
	};
}

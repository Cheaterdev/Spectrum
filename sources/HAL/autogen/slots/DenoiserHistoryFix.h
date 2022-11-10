#pragma once
#include "..\Tables\DenoiserHistoryFix.h"
namespace Slots {
	struct DenoiserHistoryFix:public DataHolder<DenoiserHistoryFix, SlotID::DenoiserHistoryFix,Table::DenoiserHistoryFix,DefaultLayout::Instance2>
	{
		DenoiserHistoryFix() = default;
	};
}

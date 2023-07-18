#pragma once
#include "..\Tables\DenoiserShadow_FilterLast.h"
namespace Slots {
	struct DenoiserShadow_FilterLast:public DataHolder<DenoiserShadow_FilterLast, SlotID::DenoiserShadow_FilterLast,Table::DenoiserShadow_FilterLast,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserShadow_FilterLast() = default;
	};
}

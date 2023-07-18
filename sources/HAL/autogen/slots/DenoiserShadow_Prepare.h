#pragma once
#include "..\Tables\DenoiserShadow_Prepare.h"
namespace Slots {
	struct DenoiserShadow_Prepare:public DataHolder<DenoiserShadow_Prepare, SlotID::DenoiserShadow_Prepare,Table::DenoiserShadow_Prepare,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserShadow_Prepare() = default;
	};
}

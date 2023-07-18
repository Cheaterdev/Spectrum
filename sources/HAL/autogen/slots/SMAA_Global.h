#pragma once
#include "..\Tables\SMAA_Global.h"
namespace Slots {
	struct SMAA_Global:public DataHolder<SMAA_Global, SlotID::SMAA_Global,Table::SMAA_Global,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		SMAA_Global() = default;
	};
}

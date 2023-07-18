#pragma once
#include "..\Tables\PSSMDataGlobal.h"
namespace Slots {
	struct PSSMDataGlobal:public DataHolder<PSSMDataGlobal, SlotID::PSSMDataGlobal,Table::PSSMDataGlobal,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		PSSMDataGlobal() = default;
	};
}

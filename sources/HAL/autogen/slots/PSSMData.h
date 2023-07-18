#pragma once
#include "..\Tables\PSSMData.h"
namespace Slots {
	struct PSSMData:public DataHolder<PSSMData, SlotID::PSSMData,Table::PSSMData,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		PSSMData() = default;
	};
}

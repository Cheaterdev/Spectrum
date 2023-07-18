#pragma once
#include "..\Tables\PSSMConstants.h"
namespace Slots {
	struct PSSMConstants:public DataHolder<PSSMConstants, SlotID::PSSMConstants,Table::PSSMConstants,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		PSSMConstants() = default;
	};
}

#pragma once
#include "..\Tables\PSSMConstants.h"
namespace Slots {
	struct PSSMConstants:public DataHolder<PSSMConstants, SlotID::PSSMConstants,Table::PSSMConstants,DefaultLayout::Instance0>
	{
		PSSMConstants() = default;
	};
}

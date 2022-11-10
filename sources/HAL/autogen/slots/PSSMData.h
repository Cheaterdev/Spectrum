#pragma once
#include "..\Tables\PSSMData.h"
namespace Slots {
	struct PSSMData:public DataHolder<PSSMData, SlotID::PSSMData,Table::PSSMData,DefaultLayout::Instance1>
	{
		PSSMData() = default;
	};
}

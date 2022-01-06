#pragma once
#include "..\Tables\SMAA_Weights.h"
namespace Slots {
	struct SMAA_Weights:public DataHolder<SMAA_Weights, SlotID::SMAA_Weights,Table::SMAA_Weights,DefaultLayout::Instance1>
	{
		SMAA_Weights() = default;
	};
}

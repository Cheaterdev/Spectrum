#pragma once
#include "..\Tables\DenoiserDownsample.h"
namespace Slots {
	struct DenoiserDownsample:public DataHolder<DenoiserDownsample, SlotID::DenoiserDownsample,Table::DenoiserDownsample,DefaultLayout::Instance2>
	{
		DenoiserDownsample() = default;
	};
}

#pragma once
#include "..\Tables\DenoiserDownsample.h"
namespace Slots {
	struct DenoiserDownsample:public DataHolder<DenoiserDownsample, SlotID::DenoiserDownsample,Table::DenoiserDownsample,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserDownsample() = default;
	};
}

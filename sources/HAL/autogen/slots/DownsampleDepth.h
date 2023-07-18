#pragma once
#include "..\Tables\DownsampleDepth.h"
namespace Slots {
	struct DownsampleDepth:public DataHolder<DownsampleDepth, SlotID::DownsampleDepth,Table::DownsampleDepth,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DownsampleDepth() = default;
	};
}

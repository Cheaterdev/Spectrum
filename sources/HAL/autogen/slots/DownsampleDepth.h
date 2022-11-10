#pragma once
#include "..\Tables\DownsampleDepth.h"
namespace Slots {
	struct DownsampleDepth:public DataHolder<DownsampleDepth, SlotID::DownsampleDepth,Table::DownsampleDepth,DefaultLayout::Instance0>
	{
		DownsampleDepth() = default;
	};
}

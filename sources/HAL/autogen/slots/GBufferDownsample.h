#pragma once
#include "..\Tables\GBufferDownsample.h"
namespace Slots {
	struct GBufferDownsample:public DataHolder<GBufferDownsample, SlotID::GBufferDownsample,Table::GBufferDownsample,DefaultLayout::Instance2>
	{
		GBufferDownsample() = default;
	};
}

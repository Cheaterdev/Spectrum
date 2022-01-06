#pragma once
#include "..\Tables\GBufferQuality.h"
namespace Slots {
	struct GBufferQuality:public DataHolder<GBufferQuality, SlotID::GBufferQuality,Table::GBufferQuality,DefaultLayout::Instance2>
	{
		GBufferQuality() = default;
	};
}

#pragma once
#include "..\Tables\NinePatch.h"
namespace Slots {
	struct NinePatch:public DataHolder<NinePatch, SlotID::NinePatch,Table::NinePatch,DefaultLayout::Instance0>
	{
		NinePatch() = default;
	};
}

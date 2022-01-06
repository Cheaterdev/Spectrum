#pragma once
#include "..\Tables\FSR.h"
namespace Slots {
	struct FSR:public DataHolder<FSR, SlotID::FSR,Table::FSR,DefaultLayout::Instance0>
	{
		FSR() = default;
	};
}

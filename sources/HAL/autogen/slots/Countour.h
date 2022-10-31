#pragma once
#include "..\Tables\Countour.h"
namespace Slots {
	struct Countour:public DataHolder<Countour, SlotID::Countour,Table::Countour,DefaultLayout::Instance0>
	{
		Countour() = default;
	};
}

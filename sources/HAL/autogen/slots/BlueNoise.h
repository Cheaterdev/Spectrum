#pragma once
#include "..\Tables\BlueNoise.h"
namespace Slots {
	struct BlueNoise:public DataHolder<BlueNoise, SlotID::BlueNoise,Table::BlueNoise,DefaultLayout::Instance0>
	{
		BlueNoise() = default;
	};
}

#pragma once
#include "..\Tables\PickerBuffer.h"
namespace Slots {
	struct PickerBuffer:public DataHolder<SlotID::PickerBuffer,Table::PickerBuffer,DefaultLayout::Instance0>
	{
		UAV uav;
		PickerBuffer(): DataHolder(uav){}
		PickerBuffer(const PickerBuffer&other): DataHolder(uav){uav = other.uav;}
	};
}

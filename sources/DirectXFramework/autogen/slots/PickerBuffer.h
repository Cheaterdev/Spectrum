#pragma once
#include "..\Tables\PickerBuffer.h"
namespace Slots {
	struct PickerBuffer:public DataHolder<PickerBuffer, SlotID::PickerBuffer,Table::PickerBuffer,DefaultLayout::Instance0>
	{
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::UAV_ID};
		PickerBuffer(): DataHolder(uav){}
		PickerBuffer(const PickerBuffer&other): DataHolder(uav){uav = other.uav;}
	};
}

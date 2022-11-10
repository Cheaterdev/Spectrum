#pragma once
#include "..\Tables\PickerBuffer.h"
namespace Slots {
	struct PickerBuffer:public DataHolder<PickerBuffer, SlotID::PickerBuffer,Table::PickerBuffer,DefaultLayout::Instance0>
	{
		PickerBuffer() = default;
	};
}

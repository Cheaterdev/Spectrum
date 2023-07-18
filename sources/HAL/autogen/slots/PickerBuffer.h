#pragma once
#include "..\Tables\PickerBuffer.h"
namespace Slots {
	struct PickerBuffer:public DataHolder<PickerBuffer, SlotID::PickerBuffer,Table::PickerBuffer,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		PickerBuffer() = default;
	};
}

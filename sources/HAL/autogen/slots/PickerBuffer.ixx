export module HAL:Autogen.Slots.PickerBuffer;
import Core;
import :Autogen.Tables.PickerBuffer;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct PickerBuffer:public DataHolder<PickerBuffer, SlotID::PickerBuffer,Table::PickerBuffer,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		PickerBuffer() = default;
	};
}

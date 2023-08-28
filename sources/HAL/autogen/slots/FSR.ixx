export module HAL:Autogen.Slots.FSR;
import Core;
import :Autogen.Tables.FSR;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct FSR:public DataHolder<FSR, SlotID::FSR,Table::FSR,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FSR() = default;
	};
}

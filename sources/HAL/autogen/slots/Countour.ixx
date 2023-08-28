export module HAL:Autogen.Slots.Countour;
import Core;
import :Autogen.Tables.Countour;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct Countour:public DataHolder<Countour, SlotID::Countour,Table::Countour,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		Countour() = default;
	};
}

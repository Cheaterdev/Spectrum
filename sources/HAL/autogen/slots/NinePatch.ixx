export module HAL:Autogen.Slots.NinePatch;
import Core;
import :Autogen.Tables.NinePatch;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct NinePatch:public DataHolder<NinePatch, SlotID::NinePatch,Table::NinePatch,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		NinePatch() = default;
	};
}

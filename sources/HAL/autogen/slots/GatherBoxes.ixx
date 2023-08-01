export module HAL:Autogen.Slots.GatherBoxes;
import Core;
import :Autogen.Tables.GatherBoxes;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct GatherBoxes:public DataHolder<GatherBoxes, SlotID::GatherBoxes,Table::GatherBoxes,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		GatherBoxes() = default;
	};
}

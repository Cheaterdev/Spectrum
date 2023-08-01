export module HAL:Autogen.Slots.PSSMLighting;
import Core;
import :Autogen.Tables.PSSMLighting;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct PSSMLighting:public DataHolder<PSSMLighting, SlotID::PSSMLighting,Table::PSSMLighting,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		PSSMLighting() = default;
	};
}

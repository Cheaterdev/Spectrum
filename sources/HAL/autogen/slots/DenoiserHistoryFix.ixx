export module HAL:Autogen.Slots.DenoiserHistoryFix;
import Core;
import :Autogen.Tables.DenoiserHistoryFix;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct DenoiserHistoryFix:public DataHolder<DenoiserHistoryFix, SlotID::DenoiserHistoryFix,Table::DenoiserHistoryFix,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserHistoryFix() = default;
	};
}

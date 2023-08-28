export module HAL:Autogen.Slots.DenoiserShadow_Filter;
import Core;
import :Autogen.Tables.DenoiserShadow_Filter;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct DenoiserShadow_Filter:public DataHolder<DenoiserShadow_Filter, SlotID::DenoiserShadow_Filter,Table::DenoiserShadow_Filter,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserShadow_Filter() = default;
	};
}

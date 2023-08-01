export module HAL:Autogen.Slots.DenoiserShadow_Prepare;
import Core;
import :Autogen.Tables.DenoiserShadow_Prepare;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct DenoiserShadow_Prepare:public DataHolder<DenoiserShadow_Prepare, SlotID::DenoiserShadow_Prepare,Table::DenoiserShadow_Prepare,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserShadow_Prepare() = default;
	};
}

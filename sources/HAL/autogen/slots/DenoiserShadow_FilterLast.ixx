export module HAL:Autogen.Slots.DenoiserShadow_FilterLast;
import Core;
import :Autogen.Tables.DenoiserShadow_FilterLast;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct DenoiserShadow_FilterLast:public DataHolder<DenoiserShadow_FilterLast, SlotID::DenoiserShadow_FilterLast,Table::DenoiserShadow_FilterLast,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserShadow_FilterLast() = default;
	};
}

export module HAL:Autogen.Slots.DenoiserShadow_FilterLocal;
import Core;
import :Autogen.Tables.DenoiserShadow_FilterLocal;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct DenoiserShadow_FilterLocal:public DataHolder<DenoiserShadow_FilterLocal, SlotID::DenoiserShadow_FilterLocal,Table::DenoiserShadow_FilterLocal,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DenoiserShadow_FilterLocal() = default;
	};
}

export module HAL:Autogen.Slots.SMAA_Global;
import Core;
import :Autogen.Tables.SMAA_Global;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct SMAA_Global:public DataHolder<SMAA_Global, SlotID::SMAA_Global,Table::SMAA_Global,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		SMAA_Global() = default;
	};
}

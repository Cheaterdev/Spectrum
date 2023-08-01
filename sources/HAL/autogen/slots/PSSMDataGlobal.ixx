export module HAL:Autogen.Slots.PSSMDataGlobal;
import Core;
import :Autogen.Tables.PSSMDataGlobal;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct PSSMDataGlobal:public DataHolder<PSSMDataGlobal, SlotID::PSSMDataGlobal,Table::PSSMDataGlobal,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		PSSMDataGlobal() = default;
	};
}

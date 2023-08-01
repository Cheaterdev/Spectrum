export module HAL:Autogen.Slots.PSSMData;
import Core;
import :Autogen.Tables.PSSMData;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct PSSMData:public DataHolder<PSSMData, SlotID::PSSMData,Table::PSSMData,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		PSSMData() = default;
	};
}

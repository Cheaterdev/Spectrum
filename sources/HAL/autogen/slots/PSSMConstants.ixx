export module HAL:Autogen.Slots.PSSMConstants;
import Core;
import :Autogen.Tables.PSSMConstants;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct PSSMConstants:public DataHolder<PSSMConstants, SlotID::PSSMConstants,Table::PSSMConstants,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		PSSMConstants() = default;
	};
}

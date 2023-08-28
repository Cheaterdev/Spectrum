export module HAL:Autogen.Slots.SMAA_Weights;
import Core;
import :Autogen.Tables.SMAA_Weights;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct SMAA_Weights:public DataHolder<SMAA_Weights, SlotID::SMAA_Weights,Table::SMAA_Weights,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		SMAA_Weights() = default;
	};
}

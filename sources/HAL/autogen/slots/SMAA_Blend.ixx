export module HAL:Autogen.Slots.SMAA_Blend;
import Core;
import :Autogen.Tables.SMAA_Blend;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct SMAA_Blend:public DataHolder<SMAA_Blend, SlotID::SMAA_Blend,Table::SMAA_Blend,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		SMAA_Blend() = default;
	};
}

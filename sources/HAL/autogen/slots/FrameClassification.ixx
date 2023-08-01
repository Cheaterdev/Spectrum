export module HAL:Autogen.Slots.FrameClassification;
import Core;
import :Autogen.Tables.FrameClassification;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct FrameClassification:public DataHolder<FrameClassification, SlotID::FrameClassification,Table::FrameClassification,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FrameClassification() = default;
	};
}

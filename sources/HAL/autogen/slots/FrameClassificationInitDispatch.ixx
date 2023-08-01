export module HAL:Autogen.Slots.FrameClassificationInitDispatch;
import Core;
import :Autogen.Tables.FrameClassificationInitDispatch;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct FrameClassificationInitDispatch:public DataHolder<FrameClassificationInitDispatch, SlotID::FrameClassificationInitDispatch,Table::FrameClassificationInitDispatch,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FrameClassificationInitDispatch() = default;
	};
}

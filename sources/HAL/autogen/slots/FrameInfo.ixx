export module HAL:Autogen.Slots.FrameInfo;
import Core;
import :Autogen.Tables.FrameInfo;
import :Autogen.Layouts.FrameLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct FrameInfo:public DataHolder<FrameInfo, SlotID::FrameInfo,Table::FrameInfo,FrameLayout::CameraData>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FrameInfo() = default;
	};
}

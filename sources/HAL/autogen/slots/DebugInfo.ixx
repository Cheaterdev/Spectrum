export module HAL:Autogen.Slots.DebugInfo;
import Core;
import :Autogen.Tables.DebugInfo;
import :Autogen.Layouts.FrameLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct DebugInfo:public DataHolder<DebugInfo, SlotID::DebugInfo,Table::DebugInfo,FrameLayout::DebugInfo>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DebugInfo() = default;
	};
}

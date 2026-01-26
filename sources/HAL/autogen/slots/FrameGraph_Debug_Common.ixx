export module HAL:Autogen.Slots.FrameGraph_Debug_Common;
import Core;
import :Autogen.Tables.FrameGraph_Debug_Common;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;

export namespace Slots 
{
	struct FrameGraph_Debug_Common :public DataHolder<FrameGraph_Debug_Common, SlotID::FrameGraph_Debug_Common, Table::FrameGraph_Debug_Common, DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FrameGraph_Debug_Common() = default;
	};
}
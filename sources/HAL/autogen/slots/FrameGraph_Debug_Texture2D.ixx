export module HAL:Autogen.Slots.FrameGraph_Debug_Texture2D;
import Core;
import :Autogen.Tables.FrameGraph_Debug_Texture2D;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;

export namespace Slots 
{
	struct FrameGraph_Debug_Texture2D :public DataHolder<FrameGraph_Debug_Texture2D, SlotID::FrameGraph_Debug_Texture2D, Table::FrameGraph_Debug_Texture2D, DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FrameGraph_Debug_Texture2D() = default;
	};
}
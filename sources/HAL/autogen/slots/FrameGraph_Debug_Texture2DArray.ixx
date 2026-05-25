export module HAL:Autogen.Slots.FrameGraph_Debug_Texture2DArray;
import Core;
import :Autogen.Tables.FrameGraph_Debug_Texture2DArray;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;

export namespace Slots 
{
	struct FrameGraph_Debug_Texture2DArray :public DataHolder<FrameGraph_Debug_Texture2DArray, SlotID::FrameGraph_Debug_Texture2DArray, Table::FrameGraph_Debug_Texture2DArray, DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FrameGraph_Debug_Texture2DArray() = default;
	};
}
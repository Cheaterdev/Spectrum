export module HAL:Autogen.Slots.FrameGraph_Debug_TextureCube;
import Core;
import :Autogen.Tables.FrameGraph_Debug_TextureCube;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;

export namespace Slots 
{
	struct FrameGraph_Debug_TextureCube :public DataHolder<FrameGraph_Debug_TextureCube, SlotID::FrameGraph_Debug_TextureCube, Table::FrameGraph_Debug_TextureCube, DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FrameGraph_Debug_TextureCube() = default;
	};
}
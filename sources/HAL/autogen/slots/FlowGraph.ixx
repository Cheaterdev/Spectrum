export module HAL:Autogen.Slots.FlowGraph;
import Core;
import :Autogen.Tables.FlowGraph;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct FlowGraph:public DataHolder<FlowGraph, SlotID::FlowGraph,Table::FlowGraph,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FlowGraph() = default;
	};
}

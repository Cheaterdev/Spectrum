export module HAL:Autogen.Slots.GraphInput;
import Core;
import :Autogen.Tables.GraphInput;
import :Autogen.Layouts.NoneLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;

export namespace Slots 
{
	struct GraphInput :public DataHolder<GraphInput, SlotID::GraphInput, Table::GraphInput, NoneLayout::None>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		GraphInput() = default;
	};
}
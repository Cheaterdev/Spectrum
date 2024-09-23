export module HAL:Autogen.Slots.WorkGraphTest;
import Core;
import :Autogen.Tables.WorkGraphTest;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;

export namespace Slots 
{
	struct WorkGraphTest :public DataHolder<WorkGraphTest, SlotID::WorkGraphTest, Table::WorkGraphTest, DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		WorkGraphTest() = default;
	};
}
export module HAL:Autogen.Slots.VoxelOutput;
import Core;
import :Autogen.Tables.VoxelOutput;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct VoxelOutput:public DataHolder<VoxelOutput, SlotID::VoxelOutput,Table::VoxelOutput,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelOutput() = default;
	};
}

export module HAL:Autogen.Slots.VoxelCopy;
import Core;
import :Autogen.Tables.VoxelCopy;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct VoxelCopy:public DataHolder<VoxelCopy, SlotID::VoxelCopy,Table::VoxelCopy,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelCopy() = default;
	};
}

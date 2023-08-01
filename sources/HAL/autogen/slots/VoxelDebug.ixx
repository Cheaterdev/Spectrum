export module HAL:Autogen.Slots.VoxelDebug;
import Core;
import :Autogen.Tables.VoxelDebug;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct VoxelDebug:public DataHolder<VoxelDebug, SlotID::VoxelDebug,Table::VoxelDebug,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelDebug() = default;
	};
}

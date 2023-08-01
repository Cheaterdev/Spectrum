export module HAL:Autogen.Slots.VoxelVisibility;
import Core;
import :Autogen.Tables.VoxelVisibility;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct VoxelVisibility:public DataHolder<VoxelVisibility, SlotID::VoxelVisibility,Table::VoxelVisibility,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelVisibility() = default;
	};
}

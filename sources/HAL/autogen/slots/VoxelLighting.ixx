export module HAL:Autogen.Slots.VoxelLighting;
import Core;
import :Autogen.Tables.VoxelLighting;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct VoxelLighting:public DataHolder<VoxelLighting, SlotID::VoxelLighting,Table::VoxelLighting,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelLighting() = default;
	};
}

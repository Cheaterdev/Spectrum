export module HAL:Autogen.Slots.VoxelScreen;
import Core;
import :Autogen.Tables.VoxelScreen;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct VoxelScreen:public DataHolder<VoxelScreen, SlotID::VoxelScreen,Table::VoxelScreen,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelScreen() = default;
	};
}

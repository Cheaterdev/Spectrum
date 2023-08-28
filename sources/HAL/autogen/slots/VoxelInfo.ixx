export module HAL:Autogen.Slots.VoxelInfo;
import Core;
import :Autogen.Tables.VoxelInfo;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct VoxelInfo:public DataHolder<VoxelInfo, SlotID::VoxelInfo,Table::VoxelInfo,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelInfo() = default;
	};
}

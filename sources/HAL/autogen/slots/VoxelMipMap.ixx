export module HAL:Autogen.Slots.VoxelMipMap;
import Core;
import :Autogen.Tables.VoxelMipMap;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct VoxelMipMap:public DataHolder<VoxelMipMap, SlotID::VoxelMipMap,Table::VoxelMipMap,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelMipMap() = default;
	};
}

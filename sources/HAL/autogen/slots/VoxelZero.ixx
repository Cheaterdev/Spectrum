export module HAL:Autogen.Slots.VoxelZero;
import Core;
import :Autogen.Tables.VoxelZero;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct VoxelZero:public DataHolder<VoxelZero, SlotID::VoxelZero,Table::VoxelZero,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelZero() = default;
	};
}

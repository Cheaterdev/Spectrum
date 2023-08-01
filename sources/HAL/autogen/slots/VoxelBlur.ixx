export module HAL:Autogen.Slots.VoxelBlur;
import Core;
import :Autogen.Tables.VoxelBlur;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct VoxelBlur:public DataHolder<VoxelBlur, SlotID::VoxelBlur,Table::VoxelBlur,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelBlur() = default;
	};
}

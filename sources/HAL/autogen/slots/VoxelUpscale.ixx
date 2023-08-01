export module HAL:Autogen.Slots.VoxelUpscale;
import Core;
import :Autogen.Tables.VoxelUpscale;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct VoxelUpscale:public DataHolder<VoxelUpscale, SlotID::VoxelUpscale,Table::VoxelUpscale,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		VoxelUpscale() = default;
	};
}

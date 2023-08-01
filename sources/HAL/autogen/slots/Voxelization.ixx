export module HAL:Autogen.Slots.Voxelization;
import Core;
import :Autogen.Tables.Voxelization;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct Voxelization:public DataHolder<Voxelization, SlotID::Voxelization,Table::Voxelization,DefaultLayout::Instance3>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		Voxelization() = default;
	};
}

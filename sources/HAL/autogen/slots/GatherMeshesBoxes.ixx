export module HAL:Autogen.Slots.GatherMeshesBoxes;
import Core;
import :Autogen.Tables.GatherMeshesBoxes;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct GatherMeshesBoxes:public DataHolder<GatherMeshesBoxes, SlotID::GatherMeshesBoxes,Table::GatherMeshesBoxes,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		GatherMeshesBoxes() = default;
	};
}

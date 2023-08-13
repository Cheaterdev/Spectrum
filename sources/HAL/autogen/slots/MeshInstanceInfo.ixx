export module HAL:Autogen.Slots.MeshInstanceInfo;
import Core;
import :Autogen.Tables.MeshInstanceInfo;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct MeshInstanceInfo:public DataHolder<MeshInstanceInfo, SlotID::MeshInstanceInfo,Table::MeshInstanceInfo,DefaultLayout::Instance2>
	{
		using DataHolder<MeshInstanceInfo, SlotID::MeshInstanceInfo,Table::MeshInstanceInfo,DefaultLayout::Instance2>::Slot;
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		MeshInstanceInfo() = default;
	};
}

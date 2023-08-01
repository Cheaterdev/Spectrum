export module HAL:Autogen.Slots.MeshInfo;
import Core;
import :Autogen.Tables.MeshInfo;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct MeshInfo:public DataHolder<MeshInfo, SlotID::MeshInfo,Table::MeshInfo,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		MeshInfo() = default;
	};
}

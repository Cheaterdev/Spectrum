export module HAL:Autogen.Slots.MaterialInfo;
import Core;
import :Autogen.Tables.MaterialInfo;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct MaterialInfo:public DataHolder<MaterialInfo, SlotID::MaterialInfo,Table::MaterialInfo,DefaultLayout::MaterialData>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		MaterialInfo() = default;
	};
}

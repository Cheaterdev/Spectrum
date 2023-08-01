export module HAL:Autogen.Slots.MipMapping;
import Core;
import :Autogen.Tables.MipMapping;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct MipMapping:public DataHolder<MipMapping, SlotID::MipMapping,Table::MipMapping,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		MipMapping() = default;
	};
}

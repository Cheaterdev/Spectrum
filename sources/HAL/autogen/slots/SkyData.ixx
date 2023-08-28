export module HAL:Autogen.Slots.SkyData;
import Core;
import :Autogen.Tables.SkyData;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct SkyData:public DataHolder<SkyData, SlotID::SkyData,Table::SkyData,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		SkyData() = default;
	};
}

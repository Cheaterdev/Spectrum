export module HAL:Autogen.Slots.GBufferQuality;
import Core;
import :Autogen.Tables.GBufferQuality;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct GBufferQuality:public DataHolder<GBufferQuality, SlotID::GBufferQuality,Table::GBufferQuality,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		GBufferQuality() = default;
	};
}

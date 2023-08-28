export module HAL:Autogen.Slots.GBufferDownsample;
import Core;
import :Autogen.Tables.GBufferDownsample;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct GBufferDownsample:public DataHolder<GBufferDownsample, SlotID::GBufferDownsample,Table::GBufferDownsample,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		GBufferDownsample() = default;
	};
}

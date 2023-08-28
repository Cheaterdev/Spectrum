export module HAL:Autogen.Slots.GBuffer;
import Core;
import :Autogen.Tables.GBuffer;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct GBuffer:public DataHolder<GBuffer, SlotID::GBuffer,Table::GBuffer,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		GBuffer() = default;
	};
}

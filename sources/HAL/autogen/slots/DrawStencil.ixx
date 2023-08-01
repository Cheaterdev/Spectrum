export module HAL:Autogen.Slots.DrawStencil;
import Core;
import :Autogen.Tables.DrawStencil;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct DrawStencil:public DataHolder<DrawStencil, SlotID::DrawStencil,Table::DrawStencil,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DrawStencil() = default;
	};
}

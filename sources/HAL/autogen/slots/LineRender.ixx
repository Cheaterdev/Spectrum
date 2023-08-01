export module HAL:Autogen.Slots.LineRender;
import Core;
import :Autogen.Tables.LineRender;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct LineRender:public DataHolder<LineRender, SlotID::LineRender,Table::LineRender,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		LineRender() = default;
	};
}

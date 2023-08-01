export module HAL:Autogen.Slots.ColorRect;
import Core;
import :Autogen.Tables.ColorRect;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct ColorRect:public DataHolder<ColorRect, SlotID::ColorRect,Table::ColorRect,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		ColorRect() = default;
	};
}

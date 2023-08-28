export module HAL:Autogen.Slots.Color;
import Core;
import :Autogen.Tables.Color;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct Color:public DataHolder<Color, SlotID::Color,Table::Color,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		Color() = default;
	};
}

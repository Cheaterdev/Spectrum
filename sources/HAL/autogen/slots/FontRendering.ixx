export module HAL:Autogen.Slots.FontRendering;
import Core;
import :Autogen.Tables.FontRendering;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct FontRendering:public DataHolder<FontRendering, SlotID::FontRendering,Table::FontRendering,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FontRendering() = default;
	};
}

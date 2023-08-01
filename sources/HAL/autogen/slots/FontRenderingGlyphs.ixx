export module HAL:Autogen.Slots.FontRenderingGlyphs;
import Core;
import :Autogen.Tables.FontRenderingGlyphs;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct FontRenderingGlyphs:public DataHolder<FontRenderingGlyphs, SlotID::FontRenderingGlyphs,Table::FontRenderingGlyphs,DefaultLayout::Instance2>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FontRenderingGlyphs() = default;
	};
}

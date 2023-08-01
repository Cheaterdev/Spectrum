export module HAL:Autogen.Slots.FontRenderingConstants;
import Core;
import :Autogen.Tables.FontRenderingConstants;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct FontRenderingConstants:public DataHolder<FontRenderingConstants, SlotID::FontRenderingConstants,Table::FontRenderingConstants,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		FontRenderingConstants() = default;
	};
}

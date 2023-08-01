export module HAL:Autogen.Slots.TextureRenderer;
import Core;
import :Autogen.Tables.TextureRenderer;
import :Autogen.Layouts.DefaultLayout;
import :SIG;
import :Types;
import :Enums;
import :Slots;
export namespace Slots {
	struct TextureRenderer:public DataHolder<TextureRenderer, SlotID::TextureRenderer,Table::TextureRenderer,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		TextureRenderer() = default;
	};
}

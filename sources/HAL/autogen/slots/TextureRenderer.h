#pragma once
#include "..\Tables\TextureRenderer.h"
namespace Slots {
	struct TextureRenderer:public DataHolder<TextureRenderer, SlotID::TextureRenderer,Table::TextureRenderer,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		TextureRenderer() = default;
	};
}

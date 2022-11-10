#pragma once
#include "..\Tables\TextureRenderer.h"
namespace Slots {
	struct TextureRenderer:public DataHolder<TextureRenderer, SlotID::TextureRenderer,Table::TextureRenderer,DefaultLayout::Instance0>
	{
		TextureRenderer() = default;
	};
}

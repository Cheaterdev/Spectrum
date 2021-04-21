#pragma once
#include "..\Tables\TextureRenderer.h"
namespace Slots {
	struct TextureRenderer:public DataHolder<SlotID::TextureRenderer,Table::TextureRenderer,DefaultLayout::Instance0>
	{
		SRV srv;
		TextureRenderer(): DataHolder(srv){}
		TextureRenderer(const TextureRenderer&other): DataHolder(srv){srv = other.srv;}
	};
}

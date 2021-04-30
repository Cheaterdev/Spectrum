#pragma once
#include "..\Tables\TextureRenderer.h"
namespace Slots {
	struct TextureRenderer:public DataHolder<TextureRenderer, SlotID::TextureRenderer,Table::TextureRenderer,DefaultLayout::Instance0>
	{
		SRV srv;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::SRV_ID};
		TextureRenderer(): DataHolder(srv){}
		TextureRenderer(const TextureRenderer&other): DataHolder(srv){srv = other.srv;}
	};
}

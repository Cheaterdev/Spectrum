#pragma once
#include "..\Tables\FontRenderingConstants.h"
namespace Slots {
	struct FontRenderingConstants:public DataHolder<FontRenderingConstants, SlotID::FontRenderingConstants,Table::FontRenderingConstants,DefaultLayout::Instance1>
	{
		CB cb;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::CB_ID};
		FontRenderingConstants(): DataHolder(cb){}
		FontRenderingConstants(const FontRenderingConstants&other): DataHolder(cb){cb = other.cb;}
	};
}

#pragma once
#include "..\Tables\FontRenderingConstants.h"
namespace Slots {
	struct FontRenderingConstants:public DataHolder<SlotID::FontRenderingConstants,Table::FontRenderingConstants,DefaultLayout::Instance1>
	{
		CB cb;
		FontRenderingConstants(): DataHolder(cb){}
		FontRenderingConstants(const FontRenderingConstants&other): DataHolder(cb){cb = other.cb;}
	};
}

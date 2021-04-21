#pragma once
#include "..\Tables\ColorRect.h"
namespace Slots {
	struct ColorRect:public DataHolder<SlotID::ColorRect,Table::ColorRect,DefaultLayout::Instance0>
	{
		CB cb;
		ColorRect(): DataHolder(cb){}
		ColorRect(const ColorRect&other): DataHolder(cb){cb = other.cb;}
	};
}

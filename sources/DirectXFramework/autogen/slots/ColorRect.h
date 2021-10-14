#pragma once
#include "..\Tables\ColorRect.h"
namespace Slots {
	struct ColorRect:public DataHolder<ColorRect, SlotID::ColorRect,Table::ColorRect,DefaultLayout::Instance0>
	{
		CB cb;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::CB_ID};
		ColorRect(): DataHolder(cb){}
		ColorRect(const ColorRect&other): DataHolder(cb){cb = other.cb;}
	};
}

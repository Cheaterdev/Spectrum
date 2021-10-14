#pragma once
#include "..\Tables\Color.h"
namespace Slots {
	struct Color:public DataHolder<Color, SlotID::Color,Table::Color,DefaultLayout::Instance0>
	{
		CB cb;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::CB_ID};
		Color(): DataHolder(cb){}
		Color(const Color&other): DataHolder(cb){cb = other.cb;}
	};
}
